cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(project
  main.c
)

# Create C header file with the name <pio program>.pio.h
pico_generate_pio_header(${PROJECT_NAME}  
        ${CMAKE_CURRENT_LIST_DIR}/quadrature-lib/quadrature.pio
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(project 1)
pico_enable_stdio_uart(project 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(project)

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(project PRIVATE pico_stdlib hardware_pwm hardware_uart hardware_pio)
