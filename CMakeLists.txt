cmake_minimum_required(VERSION 3.17.0)
project(ARM_PROJECT VERSION 0.1.0)

#   Project variables
set(_BINARY_DIRECTORY       ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(_SCRIPT_UTILS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/scripts/Utilities)
set(_CORTEX_CORE m0plus)

######################################################################################################
######                                                                                          ######
######                                   Executable                                             ######
######                                    Template                                              ######
######                                                                                          ######
######################################################################################################

######  Executable's Variables
#   Executable's name
set(_SAMC21_EXE_NAME "arm-samc21n")
#   Output Files
set(_SAMC21_EXE_ELF ${_SAMC21_EXE_NAME}.elf)
set(_SAMC21_EXE_BIN ${_SAMC21_EXE_NAME}.bin)
set(_SAMC21_EXE_HEX ${_SAMC21_EXE_NAME}.hex)
set(_SAMC21_EXE_ASM ${_SAMC21_EXE_NAME}.asm)
set(_SAMC21_EXE_MAP ${_SAMC21_EXE_NAME}.map)
set(_SAMC21_EXE_SYM ${_SAMC21_EXE_NAME}.sym)

#   Directories
set(_SAMC21_EXE_ELF_FILE ${_BINARY_DIRECTORY}/elf/${_SAMC21_EXE_ELF})
#   Linker Script
set(_SAMC21_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/mcu/ATSAMC21N18A/linker/samc21n18a_flash.ld)
#   OpenOCD Configuration
set(_SAMC21_OPENOCD_CFG ${CMAKE_CURRENT_SOURCE_DIR}/tools/openocd/0.10.0-13/scripts/board/atmel_samc21_xplained_pro.cfg)
#Use Nanolib
set(_USENANO 1)

#   Executable's source files
set(_SAMC21_SOURCES 
                    src/main.c
                    #src/tests/led_button/led_button.c
                    #src/tests/timer_blink/timer_blink.c
                    src/tests/usart_serial/usart.c
                    mcu/ATSAMC21N18A/start_up/startup_samc21.c
                    mcu/ATSAMC21N18A/start_up/system_samc21.c)

######  Target creation
add_executable(${_SAMC21_EXE_ELF} ${_SAMC21_SOURCES})

#   Target compiler pre-processor definitions
add_compile_definitions(${_SAMC21_EXE_ELF} 
                        __SAMC21N18A__
                        DONT_USE_CMSIS_INIT)

#   Target output directories
set_target_properties(  ${_SAMC21_EXE_ELF}
                        PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY ${_BINARY_DIRECTORY}/elf
                        LIBRARY_OUTPUT_DIRECTORY ${_BINARY_DIRECTORY}/lib
                        ARCHIVE_OUTPUT_DIRECTORY ${_BINARY_DIRECTORY}/lib)

#   Target include/header directories
target_include_directories( ${_SAMC21_EXE_ELF}
                            PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mcu/ATSAMC21N18A/include>
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/CMSIS/5.4.0/CMSIS/Core/Include>
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/arm-none-eabi/9-2019-q4-major/include>
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/arm-none-eabi/9-2019-q4-major/lib/include>
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/arm-none-eabi/9-2019-q4-major/lib/include-fixed>
                            PRIVATE
                                ${CMAKE_CURRENT_SOURCE_DIR}/src/
                                ${CMAKE_CURRENT_SOURCE_DIR}/mcu/ATSAMC21N18A/start_up/
)

#   Target compilation flags
target_compile_options(${_SAMC21_EXE_ELF}
                        PUBLIC
                        -mcpu=cortex-${_CORTEX_CORE}
                            -mthumb
                            #-mthumb-interwork
                            -mlong-calls
                            -ffunction-sections
                            
                            -fno-diagnostics-show-caret
                            -fdata-sections
                            -funsigned-char 
                            -funsigned-bitfields
                            --std=gnu99

                            -Og
                            -g3
                            -Wall
                            $<$<BOOL:${_USENANO}>:--specs=nano.specs>
                            $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
                            $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
)

#   Target linker options
target_link_options(${_SAMC21_EXE_ELF} PUBLIC
            -mthumb
            -Wl,-Map=${_BINARY_DIRECTORY}/map/${_SAMC21_EXE_MAP}
            -Wl,--start-group
            -lm
            -Wl,--end-group
            -Wl,--gc-sections
            -mcpu=cortex-${_CORTEX_CORE}
            -T${_SAMC21_LINKER_SCRIPT}
            -mthumb
            $<$<BOOL:${_USENANO}>:--specs=nano.specs>
)


#
#   Target Custom Commands
#

#   Converts an .ELF to .BIN
add_custom_target(  objcopy-bin 
                    DEPENDS ${_SAMC21_EXE_ELF})
add_custom_command( TARGET objcopy-bin
                    USES_TERMINAL
                    COMMAND ${CMAKE_OBJCOPY} -O binary ${_SAMC21_EXE_ELF_FILE} ${_BINARY_DIRECTORY}/bin/${_SAMC21_EXE_BIN})

#   Converts an .ELF to .HEX
add_custom_target(  hex 
                    DEPENDS ${_SAMC21_EXE_ELF})
add_custom_command( TARGET hex
                    USES_TERMINAL
                    COMMAND ${CMAKE_OBJCOPY} -Oihex ${_SAMC21_EXE_ELF_FILE} ${_BINARY_DIRECTORY}/bin/${_SAMC21_EXE_HEX})                    

#   Flashes the MCU with executable
add_custom_target(  flash 
                    DEPENDS ${_SAMC21_EXE_ELF})
add_custom_command( TARGET flash
                    USES_TERMINAL
                    COMMAND openocd -f ${_SAMC21_OPENOCD_CFG} -c \"program \"${_SAMC21_EXE_ELF_FILE}\" verify reset exit\")

#   Prints executable binary size to output
add_custom_target(  size 
                    DEPENDS ${_SAMC21_EXE_ELF})
add_custom_command( TARGET size
                    USES_TERMINAL
                    COMMAND ${_SCRIPT_UTILS_DIRECTORY}/Size/executable/PrettySize.exe ${_SAMC21_EXE_ELF_FILE} 32768 262144)
                    #COMMAND size ${_SAMC21_EXE_ELF_FILE})

#   Generates approximiation of assembly code based on elf
add_custom_target(  disassemble 
                    DEPENDS ${_SAMC21_ELF})
add_custom_command( TARGET disassemble
                    USES_TERMINAL
                    COMMAND ${CMAKE_OBJDUMP} -C -I -d -S ${_SAMC21_EXE_ELF_FILE} > ${_BINARY_DIRECTORY}/disassembly/${_SAMC21_EXE_ASM})

#   Prints ELF Header Info to output
add_custom_target(  elf-header 
                    DEPENDS ${_SAMC21_EXE_ELF})
add_custom_command( TARGET elf-header
                    USES_TERMINAL
                    COMMAND ${CMAKE_READELF} -h ${_SAMC21_EXE_ELF_FILE})

#   Generates symbol files
add_custom_target(  symbols 
                    DEPENDS ${_SAMC21_EXE_ELF})
add_custom_command( TARGET symbols
                    USES_TERMINAL
                    COMMAND ${CMAKE_NM} -C -l -n -S ${_SAMC21_EXE_ELF_FILE} > ${_BINARY_DIRECTORY}/sym/${_SAMC21_EXE_SYM})
