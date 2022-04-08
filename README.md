# EX2_BOOTLOADER
Welcome 👋 - this is the central repository for the Ex-alta 2 Bootloader. This code runs on our open source board dubbed 'Athena' 

The software in this repository contains a small subset of the OBC software capabilities to be placed in the first few sectors of flash on the MCU. This bootloader is fairly complex because it is difficult to access the OBC once it is on orbit due to its altitude. Good diagnostic and reliable software update capabilities need to be implemented.

### File Structure
* libcsp/
	* Submodule for the for the CSP network software.
* main/
	* Main entry point, and LEOP sequence. All background tasks and third-party systems (i.e. FreeRTOS Sceduler, CSP node) are initialized here.
* source/
	* HalCoGEN generated source files including hardware drivers, and configurations
* bl_\*
	* Files related to bootloader-specific functionality

