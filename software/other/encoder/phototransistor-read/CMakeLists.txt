cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(project
  main.c
)

pico_enable_stdio_usb(project 1)
pico_enable_stdio_uart(project 0)
pico_add_extra_outputs(project)

target_link_libraries(project PRIVATE pico_stdlib hardware_pwm hardware_uart)
