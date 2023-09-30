cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD seeed_xiao_rp2040)
set(PICO_SDK_PATH pico-sdk)
set(PICO_SDK_INIT_CMAKE_FILE ${PICO_SDK_PATH}/pico_sdk_init.cmake)

include(${PICO_SDK_INIT_CMAKE_FILE})

project(example-app-seeed-xiao-rp2040 C CXX ASM)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(app
    main.c
    )

# pull in common dependencies
target_link_libraries(app pico_stdlib)

# create map/bin/hex file etc.
pico_add_extra_outputs(app)

# Redirect STDOUT and STDIN to USB CDC
pico_enable_stdio_usb(app 1)
pico_enable_stdio_uart(app 0)
