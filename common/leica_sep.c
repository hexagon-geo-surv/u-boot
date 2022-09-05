// SPDX-License-Identifier: GPL-2.0+
// Copyright (C) 2022 Leica Geosystems AG

#include <common.h>
#include <console.h>
#include <dm.h>
#include <leica_sep.h>
#include <serial.h>

/*
 * leica_sep_init
 *
 * one time search and initialization of uart udevice driver marked in
 * the devicetree to be used for SEP commands
 *
 * @dev: return pointer for udevice-driver struct to the SEP serial device
 * @return: 0 on success
 */
static bool leica_sep_init_done = false;
static struct udevice **leica_sep_udevice;
int leica_sep_init(struct udevice **dev) {
    if (leica_sep_init_done) {
        *dev = *leica_sep_udevice;
        return 0;
    }

    int ret = 0;

#if 0
    ret = uclass_get_device_by_name(UCLASS_SERIAL,
                                    "serial@30a60000",
                                    leica_sep_udevice);
#else
    //TODO: this seems tooo roundabout... looking for a property and
    //from there getting it's enclosing node?! there seems to be no
    //"ofnode_by_label" - which would make things easier. and
    //"by_phandle" doesn't cut it either :-|
    ofnode n = ofnode_by_prop_value(
                                    ofnode_root(),
                                    "leica-MCU-uart",
                                    NULL, 0
                                    );

    ret = uclass_get_device_by_ofnode(UCLASS_SERIAL, n, leica_sep_udevice);
#endif
    if (ret) {
        log_err("%s: unable to get_device_by_ofnode (%i)\n", __FUNCTION__, ret);
        return -ENOTTY;
    }
    printf("%s: leica-sep-node -> %s\n", __FUNCTION__, dev_read_name(*leica_sep_udevice));


    printf("%s setting up uart seq_=%i\n", __FUNCTION__, (*leica_sep_udevice)->seq_);
    init_uart_clk((*leica_sep_udevice)->seq_);

    struct dm_serial_ops *ops = serial_get_ops(*leica_sep_udevice);
    if (!ops | !ops->setbrg)
        return -ENOTTY;

    //Note: that setting the baudrate also software-resets the uart block
    ops->setbrg(*leica_sep_udevice, 19200);//TODO: MCU will switch to 115200);

    leica_sep_init_done = true;

    printf("%s done\n", __FUNCTION__);
    return 0;
}


/*
 * leica_sep_transfer
 *
 * Send SEP command 'SCXXX:\r\n'
 * Wait for reception of 'SRXXX,CRC:<payload>\r\n'
 * Fills the input 'data' with the <payload>
 *
 * @cmd: SEP command (without '\r\n')
 * @data: pointer for returened payload data
 * @data_size: size of the data buffer
 * @return length >=0 number of bytes of the received payload; -ve on error
 */
int leica_sep_transfer(char *cmd,
                       char *data, size_t data_size,
                       ulong timeout_ms)
{
    struct udevice *dev;
    size_t bytes_received = 0;
    ulong start = get_timer(0);
    int cmd_length = 0;
    bool header_received = false, command_received = false;

    if (leica_sep_init(&dev))
        return -ENOTTY;

    struct dm_serial_ops *ops = serial_get_ops(dev);

    if (!ops || !ops->putc || !ops->getc || !ops->pending) {
        debug("missing necessary 'ops' on serial device\n");
        return -ENOTTY;
    }

    if (!cmd || !data) {
        debug("invalid cmd or data memory locations\n");
        return -EINVAL;
    }

    if (cmd[0] != 'S' || cmd[1] != 'C') {
        printf("input SEP command malformed; command does not start with 'SC'\n");
        return -EINVAL;
    }

    while (cmd[cmd_length] != ':' && cmd[cmd_length] != '\0') {
        cmd_length++;
    }

#if defined(DEBUG) //REMOVEME - or keep as debug feature?
    // reset output buffer to a known pattern
    for (bytes_received = 0; bytes_received < data_size; bytes_received++) {
        if (bytes_received % 2 == 0)
            data[bytes_received] = '^';
        else
            data[bytes_received] = 'v';
    }
    bytes_received = 0;
#endif

    // Empty receiver FIFO before sending our command
    while (ops->pending(dev, true) > 0) {
        if (bytes_received < data_size)
            data[bytes_received++] = ops->getc(dev);
        else
            ops->getc(dev);
    }
#if defined(DEBUG)
    if (bytes_received > 0) {
        debug("discarded bytes: %s\n", data);
    }
#endif
    bytes_received = 0;

    //NOTE: the pre-command '\r\n' is a workaround, otherwise the MCU
    //      does not process/answer to the transmited SEP-command
    while(ops->putc(dev, '\r') < 0);
    while(ops->putc(dev, '\n') < 0);
    char *s = cmd;
    while(*s){
        while(ops->putc(dev, *s++) < 0);
    }
    while(ops->putc(dev, '\r') < 0);
    while(ops->putc(dev, '\n') < 0);

    while(true)
    {
        if (get_timer(0) > (start + timeout_ms)) {
            //TODO: handle start+timer overflowing long?
            return -ETIME;
        }

        // wait for character in the RX queue
        if (!ops->pending(dev, true)) {
            udelay(4);
            continue;
        }

        // output buffer overflow
        if (bytes_received >= data_size) {
            debug("bytes_received (%li) >= data_size (%li)\n", bytes_received, data_size);
            break;
        }

        data[bytes_received] = ops->getc(dev);
        // should not happen, since 'pending' told us that there is sth to read
        if (data[bytes_received] == -EAGAIN)
            continue;

        if (data[bytes_received] == '\n' &&
            bytes_received >= 1 &&
            data[bytes_received - 1] == '\r') {

            if (header_received) {
                bytes_received--;

                //TODO: what's with the quotes?
                // should this only drop encasing ones? like SR123,crc:"some-return"
                // 'cause with ">=2" all quotes would be skipped... which might be too much?
                // -> only skip if header was *just* received, and this is the first char after
                if (bytes_received >= 2 &&
                    data[bytes_received - 2] == '"')
                  bytes_received--;

                command_received = true;
                break;
            }

            bytes_received = 0;
        }

        // scan the response for the first ':', marking the end of the response-header + crc
        if ((data[bytes_received] == ':')
          && !header_received ) {
            /* Compare data with cmd. Data before ":" can be longer
             * than cmd (including the CRC), this is why cmd_length
             * is used for the comparison
             */
            cmd[1] = 'R'; // SEP command starts with 'SC', the response with 'SR'
            if (bytes_received >= cmd_length - 1 &&
                !strncmp(data, cmd, cmd_length)) {
                header_received = true;
            }
            cmd[1] = 'C';// restore to passed in value
            //TODO: cut out crc from data[cmd_length..bytes_received]
            //      and verify response/payload against it

            bytes_received = 0;
            continue;
        }

        if (bytes_received == 0 && data[bytes_received] == '"')
            continue;

        bytes_received++;
    }

    if (!command_received) {
        debug("%s: ERR: command response not received.\n", __FUNCTION__);
        return -1;
    }

    // We substitute the last char with '\0' to terminate the payload
    // string
    data[bytes_received] = '\0';

    return bytes_received;
}
