cmake_minimum_required(VERSION 3.12)

# Change your executable name to something creative!
set(NAME ledcontroller) # <-- Name your project/executable here!

if (DEFINED ENV{PICO_SDK_PATH} AND (NOT PICO_SDK_PATH))
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    message("Using PICO_SDK_PATH from environment ('${PICO_SDK_PATH}')")
endif ()

include(pimoroni_pico_import.cmake)
include(pico_sdk_import.cmake)

# Gooey boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(${NAME})

# Add your source files
target_sources(${NAME} PUBLIC
    main.cpp
    usb_descriptors.c
    proto/types.pb.c
    nanopb/pb_common.c
    nanopb/pb_decode.c
    nanopb/pb_encode.c
)

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR} nanopb)
include(common/pimoroni_i2c)
include(drivers/plasma/plasma)
include(drivers/rgbled/rgbled)
include(drivers/analog/analog)
include(drivers/button/button)
include(libraries/plasma2040/plasma2040)

# Don't forget to link the libraries you need!
target_link_libraries(${NAME}
    pico_stdlib
    pico_unique_id
    tinyusb_device
    tinyusb_board
    plasma2040
    rgbled
    button
    analog
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

#pico_enable_stdio_usb(${NAME} 1)

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
