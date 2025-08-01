.. SPDX-License-Identifier: GPL-2.0+

Generic Distro Configuration Concept
====================================

Linux distributions are faced with supporting a variety of boot mechanisms,
environments or bootloaders (PC BIOS, EFI, U-Boot, Barebox, ...). This makes
life complicated. Worse, bootloaders such as U-Boot have a configurable set
of features, and each board chooses to enable a different set of features.
Hence, distros typically need to have board-specific knowledge in order to
set up a bootable system.

This document defines a common set of U-Boot features that are required for
a distro to support the board in a generic fashion. Any board wishing to
allow distros to install and boot in an out-of-the-box fashion should enable
all these features. Linux distros can then create a single set of boot
support/install logic that targets these features. This will allow distros
to install on many boards without the need for board-specific logic.

In fact, some of these features can be implemented by any bootloader, thus
decoupling distro install/boot logic from any knowledge of the bootloader.

This model assumes that boards will load boot configuration files from a
regular storage mechanism (eMMC, SD card, USB Disk, SATA disk, etc.) with
a standard partitioning scheme (MBR, GPT). Boards that cannot support this
storage model are outside the scope of this document, and may still need
board-specific installer/boot-configuration support in a distro.

To some extent, this model assumes that a board has a separate boot flash
that contains U-Boot, and that the user has somehow installed U-Boot to this
flash before running the distro installer. Even on boards that do not conform
to this aspect of the model, the extent of the board-specific support in the
distro installer logic would be to install a board-specific U-Boot package to
the boot partition during installation. This distro-supplied U-Boot can still
implement the same features as on any other board, and hence the distro's boot
configuration file generation logic can still be board-agnostic.

Locating Bootable Disks
-----------------------

Typical desktop/server PCs search all (or a user-defined subset of) attached
storage devices for a bootable partition, then load the bootloader or boot
configuration files from there. A U-Boot board port that enables the features
mentioned in this document will search for boot configuration files in the
same way.

Thus, distros do not need to manipulate any kind of bootloader-specific
configuration data to indicate which storage device the system should boot
from.

Distros simply need to install the boot configuration files (see next
section) in an ext2/3/4 or FAT partition, mark the partition bootable (via
the MBR bootable flag, or GPT legacy_bios_bootable attribute), and U-Boot (or
any other bootloader) will find those boot files and execute them. This is
conceptually identical to creating a grub2 configuration file on a desktop
PC.

Note that in the absence of any partition that is explicitly marked bootable,
U-Boot falls back to searching the first valid partition of a disk for boot
configuration files. Other bootloaders are recommended to do the same, since
I believe that partition table bootable flags aren't so commonly used outside
the realm of x86 PCs.

U-Boot can also search for boot configuration files from a TFTP server.

Boot Configuration Files
------------------------

The standard format for boot configuration files is that of extlinux.conf, as
handled by U-Boot's "syslinux" (disk) or "pxe boot" (network). This format is
not formally standardized and documented in a single location. However, other
implementations do document it and we attempt to be as compatible as possible.

* The UAPI Group Specifications `Boot Loader Specification`_

* The Syslinux Project documents both `PXELINUX`_ and `SYSLINUX`_ files and is
  the originator of the format.

That said, we have some differences to these documents, namely:

* Prescribes a separate configuration per boot menu option, whereas U-Boot
  lumps all options into a single extlinux.conf file. Hence, U-Boot searches
  for /extlinux/extlinux.conf then /boot/extlinux/extlinux.conf on disk, or
  pxelinux.cfg/default over the network.

* Does not document the fdtdir option, which automatically selects the DTB to
  pass to the kernel.

* If no fdt/fdtdir is provided, the U-Boot will pass its own currently used
  device tree.

* If ``-`` is passed as fdt argument and ``CONFIG_SUPPORT_PASSING_ATAGS`` is
  enabled, then no device tree will be used (legacy booting / pre-dtb kernel).

* The ``append`` string may use environment variables. For example, an
  A/B boot setup could use ``append root=PARTLABEL=root_${bootslot}``
  to set the root filesystem to the right one for the selected slot,
  assuming the ``bootslot`` environment variable is set before the
  extlinux.conf file is processed, and the partition is labeled to
  match.

See also doc/README.pxe under 'pxe file format'.

One example extlinux.conf generated by the Fedora installer is::

    # extlinux.conf generated by anaconda

    ui menu.c32

    menu autoboot Welcome to Fedora. Automatic boot in # second{,s}. Press a key for options.
    menu title Fedora Boot Options.
    menu hidden

    timeout 50
    #totaltimeout 9000

    default Fedora (3.17.0-0.rc4.git2.1.fc22.armv7hl+lpae) 22 (Rawhide)

    label Fedora (3.17.0-0.rc4.git2.1.fc22.armv7hl) 22 (Rawhide)
        kernel /boot/vmlinuz-3.17.0-0.rc4.git2.1.fc22.armv7hl
        append ro root=UUID=8eac677f-8ea8-4270-8479-d5ddbb797450 console=ttyS0,115200n8 LANG=en_US.UTF-8 drm.debug=0xf
        fdtdir /boot/dtb-3.17.0-0.rc4.git2.1.fc22.armv7hl
        initrd /boot/initramfs-3.17.0-0.rc4.git2.1.fc22.armv7hl.img

    label Fedora (3.17.0-0.rc4.git2.1.fc22.armv7hl+lpae) 22 (Rawhide)
        kernel /boot/vmlinuz-3.17.0-0.rc4.git2.1.fc22.armv7hl+lpae
        append ro root=UUID=8eac677f-8ea8-4270-8479-d5ddbb797450 console=ttyS0,115200n8 LANG=en_US.UTF-8 drm.debug=0xf
        fdtdir /boot/dtb-3.17.0-0.rc4.git2.1.fc22.armv7hl+lpae
        initrd /boot/initramfs-3.17.0-0.rc4.git2.1.fc22.armv7hl+lpae.img

    label Fedora-0-rescue-8f6ba7b039524e0eb957d2c9203f04bc (0-rescue-8f6ba7b039524e0eb957d2c9203f04bc)
        kernel /boot/vmlinuz-0-rescue-8f6ba7b039524e0eb957d2c9203f04bc
        initrd /boot/initramfs-0-rescue-8f6ba7b039524e0eb957d2c9203f04bc.img
        append ro root=UUID=8eac677f-8ea8-4270-8479-d5ddbb797450 console=ttyS0,115200n8
        fdtdir /boot/dtb-3.16.0-0.rc6.git1.1.fc22.armv7hl+lpae


One example of hand-crafted extlinux.conf::

   menu title Select kernel
   timeout 100

   label Arch with uart devicetree overlay
       kernel /arch/Image.gz
       initrd /arch/initramfs-linux.img
       fdt /dtbs/arch/board.dtb
       fdtoverlays /dtbs/arch/overlay/uart0-gpio0-1.dtbo
       append console=ttyS0,115200 console=tty1 rw root=UUID=fc0d0284-ca84-4194-bf8a-4b9da8d66908

   label Arch with uart devicetree overlay but with Boot Loader Specification keys
       kernel /arch/Image.gz
       initrd /arch/initramfs-linux.img
       devicetree /dtbs/arch/board.dtb
       devicetree-overlay /dtbs/arch/overlay/uart0-gpio0-1.dtbo
       append console=ttyS0,115200 console=tty1 rw root=UUID=fc0d0284-ca84-4194-bf8a-4b9da8d66908

Another hand-crafted network boot configuration file is::

    TIMEOUT 100

    MENU TITLE TFTP boot options

    LABEL jetson-tk1-emmc
            MENU LABEL ../zImage root on Jetson TK1 eMMC
            LINUX ../zImage
            FDTDIR ../
            APPEND console=ttyS0,115200n8 console=tty1 loglevel=8 rootwait rw earlyprintk root=PARTUUID=80a5a8e9-c744-491a-93c1-4f4194fd690b

    LABEL venice2-emmc
            MENU LABEL ../zImage root on Venice2 eMMC
            LINUX ../zImage
            FDTDIR ../
            APPEND console=ttyS0,115200n8 console=tty1 loglevel=8 rootwait rw earlyprintk root=PARTUUID=5f71e06f-be08-48ed-b1ef-ee4800cc860f

    LABEL sdcard
            MENU LABEL ../zImage, root on 2GB sdcard
            LINUX ../zImage
            FDTDIR ../
            APPEND console=ttyS0,115200n8 console=tty1 loglevel=8 rootwait rw earlyprintk root=PARTUUID=b2f82cda-2535-4779-b467-094a210fbae7

    LABEL fedora-installer-fk
            MENU LABEL Fedora installer w/ Fedora kernel
            LINUX fedora-installer/vmlinuz
            INITRD fedora-installer/initrd.img.orig
            FDTDIR fedora-installer/dtb
            APPEND loglevel=8 ip=dhcp inst.repo=http://10.0.0.2/mirrors/fedora/linux/development/rawhide/armhfp/os/ rd.shell cma=64M

U-Boot Implementation
=====================

Enabling the distro options
---------------------------

In your board's defconfig, enable the DISTRO_DEFAULTS option by adding
a line with "CONFIG_DISTRO_DEFAULTS=y". If you want to enable this
from Kconfig itself, for e.g. all boards using a specific SoC then
add a "imply DISTRO_DEFAULTS" to your SoC CONFIG option.


TO BE UPDATED:

In your board configuration file, include the following::

    #ifndef CONFIG_XPL_BUILD
    #include <config_distro_bootcmd.h>
    #endif

The first of those headers primarily enables a core set of U-Boot features,
such as support for MBR and GPT partitions, ext* and FAT filesystems, booting
raw zImage and initrd (rather than FIT- or uImage-wrapped files), etc. Network
boot support is also enabled here, which is useful in order to boot distro
installers given that distros do not commonly distribute bootable install
media for non-PC targets at present.

Finally, a few options that are mostly relevant only when using U-Boot-
specific boot.scr scripts are enabled. This enables distros to generate a
U-Boot-specific boot.scr script rather than extlinux.conf as the boot
configuration file. While doing so is fully supported, and
CONFIG_DISTRO_DEFAULTS exposes enough parameterization to boot.scr to
allow for board-agnostic boot.scr content, this document recommends that
distros generate extlinux.conf rather than boot.scr. extlinux.conf is intended
to work across multiple bootloaders, whereas boot.scr will only work with
U-Boot. TODO: document the contract between U-Boot and boot.scr re: which
environment variables a generic boot.scr may rely upon.

The second of those headers sets up the default environment so that $bootcmd
is defined in a way that searches attached disks for boot configuration files,
and executes them if found.

Required Environment Variables
------------------------------

The U-Boot "syslinux" and "pxe boot" commands require a number of environment
variables be set. Default values for these variables are often hard-coded into
CFG_EXTRA_ENV_SETTINGS in the board's U-Boot configuration file, so that
the user doesn't have to configure them.

fdt_addr:
  Mandatory for any system that provides the DTB in HW (e.g. ROM) and wishes
  to pass that DTB to Linux, rather than loading a DTB from the boot
  filesystem. Prohibited for any other system.

  If specified a DTB to boot the system must be available at the given
  address.

fdt_addr_r:
  Mandatory. The location in RAM where the DTB will be loaded or copied to when
  processing the fdtdir/devicetreedir or fdt/devicetree options in
  extlinux.conf.

  This is mandatory even when fdt_addr is provided, since extlinux.conf must
  always be able to provide a DTB which overrides any copy provided by the HW.

  A size of 1MB for the FDT/DTB seems reasonable.

fdtoverlay_addr_r:
  Mandatory. The location in RAM where DTB overlays will be temporarily
  stored and then applied in the load order to the fdt blob stored at the
  address indicated in the fdt_addr_r environment variable.

fdtfile:
  Mandatory. the name of the DTB file for the specific board for instance
  the espressobin v5 board the value is "marvell/armada-3720-espressobin.dtb"
  while on a clearfog pro it is "armada-388-clearfog-pro.dtb" in the case of
  a board providing its firmware based DTB this value can be used to override
  the DTB with a different DTB. fdtfile will automatically be set for you if
  it matches the format ${soc}-${board}.dtb which covers most 32 bit use cases.
  AArch64 generally does not match as the Linux kernel put the dtb files under
  SoC vendor directories.

ramdisk_addr_r:
  Mandatory. The location in RAM where the initial ramdisk will be loaded to
  when processing the initrd option in extlinux.conf.

  It is recommended that this location be highest in RAM out of fdt_addr_r,
  kernel_addr_r, and ramdisk_addr_r, so that the RAM disk can vary in size
  and use any available RAM.

kernel_addr_r:
  Mandatory. The location in RAM where the kernel will be loaded to when
  processing the kernel option in the extlinux.conf.

  The kernel should be located within the first 128M of RAM in order for the
  kernel CONFIG_AUTO_ZRELADDR option to work, which is likely enabled on any
  distro kernel. Since the kernel will decompress itself to 0x8000 after the
  start of RAM, kernel_addr_r should not overlap that area, or the kernel will
  have to copy itself somewhere else first before decompression.

  A size of 16MB for the kernel is likely adequate.

kernel_comp_addr_r:
  Optional. This is only required if user wants to boot Linux from a compressed
  Image(.gz, .bz2, .lzma, .lzo) using the booti command. It represents the
  location in RAM where the compressed Image will be decompressed temporarily.
  Once the decompression is complete, the decompressed data will be moved to
  kernel_addr_r for booting.

kernel_comp_size:
  Optional. This is only required if user wants to boot Linux from a compressed
  Image using booti command. It represents the size of the compressed file. The
  size has to at least the size of loaded image for decompression to succeed.

pxefile_addr_r:
  Mandatory. The location in RAM where extlinux.conf will be loaded to prior
  to processing.

  A size of 1MB for extlinux.conf is more than adequate.

scriptaddr:
  Mandatory, if the boot script is boot.scr rather than extlinux.conf. The
  location in RAM where boot.scr will be loaded to prior to execution.

  A size of 1MB for extlinux.conf is more than adequate.

For suggestions on memory locations for ARM systems, you must follow the
guidelines specified in Documentation/arm/Booting in the Linux kernel tree.

For a commented example of setting these values, please see the definition of
MEM_LAYOUT_ENV_SETTINGS in include/configs/tegra124-common.h.

Boot Target Configuration
-------------------------

The `config_distro_bootcmd.h` file defines $bootcmd and many helper command
variables that automatically search attached disks for boot configuration files
and execute them. Boards must provide configure <config_distro_bootcmd.h> so
that it supports the correct set of possible boot device types. To provide this
configuration, simply define macro BOOT_TARGET_DEVICES prior to including
<config_distro_bootcmd.h>. For example::

    #ifndef CONFIG_XPL_BUILD
    #define BOOT_TARGET_DEVICES(func) \
            func(MMC, mmc, 1) \
            func(MMC, mmc, 0) \
            func(USB, usb, 0) \
            func(PXE, pxe, na) \
            func(DHCP, dhcp, na)
    #include <config_distro_bootcmd.h>
    #endif

Each entry in the macro defines a single boot device (e.g. a specific eMMC
device or SD card) or type of boot device (e.g. USB disk). The parameters to
the func macro (passed in by the internal implementation of the header) are:

- Upper-case disk type (DHCP, HOST, IDE, MMC, NVME, PXE, SATA, SCSI, UBIFS, USB,
  VIRTIO).
- Lower-case disk type (same options as above).
- ID of the specific disk (MMC only) or ignored for other types.

User Configuration
==================

Once the user has installed U-Boot, it is expected that the environment will
be reset to the default values in order to enable $bootcmd and friends, as set
up by <config_distro_bootcmd.h>. After this, various environment variables may
be altered to influence the boot process:

boot_targets:
  The list of boot locations searched.

  Example: mmc0, mmc1, usb, pxe

  Entries may be removed or re-ordered in this list to affect the boot order.

boot_prefixes:
  For disk-based booting, the list of directories within a partition that are
  searched for boot configuration files (extlinux.conf, boot.scr).

  Example: / /boot/

  Entries may be removed or re-ordered in this list to affect the set of
  directories which are searched.

boot_scripts:
  The name of U-Boot style boot.scr files that $bootcmd searches for.

  Example: boot.scr.uimg boot.scr

  (Typically we expect extlinux.conf to be used, but execution of boot.scr is
  maintained for backwards-compatibility.)

  Entries may be removed or re-ordered in this list to affect the set of
  filenames which are supported.

scan_dev_for_extlinux:
  If you want to disable extlinux.conf on all disks, set the value to something
  innocuous, e.g. setenv scan_dev_for_extlinux true.

scan_dev_for_scripts:
  If you want to disable boot.scr on all disks, set the value to something
  innocuous, e.g. setenv scan_dev_for_scripts true.

boot_net_usb_start:
  If you want to prevent USB enumeration by distro boot commands which execute
  network operations, set the value to something innocuous, e.g. setenv
  boot_net_usb_start true. This would be useful if you know your Ethernet
  device is not attached to USB, and you wish to increase boot speed by
  avoiding unnecessary actions.

boot_net_pci_enum:
  If you want to prevent PCI enumeration by distro boot commands which execute
  network operations, set the value to something innocuous, e.g. setenv
  boot_net_pci_enum true. This would be useful if you know your Ethernet
  device is not attached to PCI, and you wish to increase boot speed by
  avoiding unnecessary actions.

Interactively booting from a specific device at the u-boot prompt
=================================================================

For interactively booting from a user-selected device at the u-boot command
prompt, the environment provides predefined bootcmd_<target> variables for
every target defined in boot_targets, which can be run be the user.

If the target is a storage device, the format of the target is always
<device type><device number>, e.g. mmc0.  Specifying the device number is
mandatory for storage devices, even if only support for a single instance
of the storage device is actually implemented.

For network targets (dhcp, pxe), only the device type gets specified;
they do not have a device number.

Examples:

 - run bootcmd_usb0
   boots from the first USB mass storage device

 - run bootcmd_mmc1
   boots from the second MMC device

 - run bootcmd_pxe
   boots by tftp using a pxelinux.cfg

The list of possible targets consists of:

- network targets

  * dhcp
  * pxe

- storage targets (to which a device number must be appended)

  * mmc
  * sata
  * scsi
  * ide
  * usb
  * virtio

Other *boot* variables than the ones defined above are only for internal use
of the boot environment and are not guaranteed to exist or work in the same
way in future u-boot versions.  In particular the <device type>_boot
variables (e.g. mmc_boot, usb_boot) are a strictly internal implementation
detail and must not be used as a public interface.

.. _`Boot Loader Specification`: https://uapi-group.org/specifications/specs/boot_loader_specification/
.. _`PXELINUX`: https://wiki.syslinux.org/wiki/index.php?title=PXELINUX
.. _`SYSLINUX`: https://wiki.syslinux.org/wiki/index.php?title=SYSLINUX

.. sectionauthor:: (C) Copyright 2014 Red Hat Inc.
.. sectionauthor:: Copyright (c) 2014-2015, NVIDIA CORPORATION.  All rights reserved.
.. sectionauthor:: Copyright (C) 2015 K. Merker <merker@debian.org>
