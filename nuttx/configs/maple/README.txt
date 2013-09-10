README
======

This README discusses issues unique to NuttX configurations for the
maple development board.

Contents
========

  - Development Environment
  - DFU

Development Environment
=======================

  Either Linux (recommended), Mac or Cygwin on Windows can be used for the development environment.
  The source has been built only using the GNU toolchain (see below).  Other
  toolchains will likely cause problems. Testing was performed using the Cygwin
  environment because the Raisonance R-Link emulatator and some RIDE7 development tools
  were used and those tools works only under Windows.

DFU
===

  The linker files in these projects can be configured to indicate that you
  will be loading code using STMicro built-in USB Device Firmware Upgrade (DFU)
  loader or via some JTAG emulator.  You can specify the DFU bootloader by
  adding the following line:

    CONFIG_STM32_DFU=y

  to your .config file. Most of the configurations in this directory are set
  up to use the DFU loader.

  If CONFIG_STM32_DFU is defined, the code will not be positioned at the beginning
  of FLASH (0x08000000) but will be offset to 0x08005000.  This offset is needed
  to make space for the DFU loader and 0x08005000 is where the DFU loader expects
  to find new applications at boot time.  If you need to change that origin for some
  other bootloader, you will need to edit the file(s) ld.script.dfu for each
  configuration. In LeafLabs case, we are using maple bootloader:

      http://leaflabs.com/docs/bootloader.html

  For Windows:

  The DFU SE PC-based software is available from the STMicro website,
  http://www.st.com.  General usage instructions:

  1. Convert the NuttX Intel Hex file (nuttx.hex) into a special DFU
     file (nuttx.dfu)... see below for details.
  2. Connect the leach board to your computer using a USB
     cable.
  3. Start the DFU loader on the maple board.  You do this by
     resetting the board while holding the "Key" button.  Windows should
     recognize that the DFU loader has been installed.
  3. Run the DFU SE program to load nuttx.dfu into FLASH.

  What if the DFU loader is not in FLASH?  The loader code is available
  inside of the Demo dirctory of the USBLib ZIP file that can be downloaded
  from the STMicro Website.  You can build it using RIDE (or other toolchains);
  you will need a JTAG emulator to burn it into FLASH the first time.

  In order to use STMicro's built-in DFU loader, you will have to get
  the NuttX binary into a special format with a .dfu extension.  The
  DFU SE PC_based software installation includes a file "DFU File Manager"
  conversion program that a file in Intel Hex format to the special DFU
  format.  When you successfully build NuttX, you will find a file called
  nutt.hex in the top-level directory.  That is the file that you should
  provide to the DFU File Manager. You will end up with a file called
  nuttx.dfu that you can use with the STMicro DFU SE program.

  For Linux or Mac:

  While on Linux or Mac, we can use dfu-util to upload nuttx binary.
  For this part, we can refer to LeafLabs official document:

    http://leaflabs.com/docs/unix-toolchain.html
