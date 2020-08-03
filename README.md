##### README #####
==================

This is a Visual Studio Code Template for an ARM project running on
Microchip ATSAMC21N18A.

Features:
    * CMake build system
    * Doxygen documentation generation
    * Cortex-Debug configuration
    * Utility command templates
    * Base working project

This template comes pre-configured for building executabled for the ATSAMC21N18A

Extensions:
------------
    The templates .code-workspace contains a number of 'recommended' extensions.

    Some of these extensions will require external tools to be installed, either on
    the system (and referenced in the PATH variable) or in the templates 'Tools' folder.

    VSCode will display them in the extensions pane as recommended, however looking
    at the workspace JSON will reveal notes on what extensions are in fact required.
    
    The required extensions are as follows:
        * C/C++ by Microsoft
        * CMake Tools by Microsoft
        * Cortex-Debug by marus25

    There are also a number of otherwise noted 'recommended' extension for 
    quality-of-life tools as well as some personal preferences of mine for otherwise
    useful extensions.

Toolchain:
----------
    In order to function at all a number of tools are required to build, debug and manage 
    this template; they are as follows:
        * ARM GNU GCC Toolchain
        * CMAKE
        * OpenOCD
    
    The following tools are recommended:
        * Doxygen
        * Git

Microcontroller Notes:
----------------------

    This project contains files specific to the ATSAMC21N18A Cortex-M0+ powered
    development board from Microchip.
    
    This template should be easily configurable for different chips/boards.

    Microcontroller specific files should be placed under the 'mcu' folder to maintain
    project structure/organisation.