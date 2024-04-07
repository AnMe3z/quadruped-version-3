cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pwm_control C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pwm_control
  main.c
)


# Create C header file with the name <pio program>.pio.h
pico_generate_pio_header(${PROJECT_NAME}  
        ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio
)

pico_enable_stdio_usb(pwm_control 1)
pico_enable_stdio_uart(pwm_control 0)
pico_add_extra_outputs(pwm_control)

target_include_directories(pwm_control PRIVATE ${CMAKE_CURRENT_LIST_DIR} )

target_link_libraries(pwm_control PRIVATE 
			pico_stdlib 
			
			hardware_pwm 
			hardware_uart
			pico_multicore
			hardware_pio
			pico_cyw43_arch_none
			pico_cyw43_arch_lwip_threadsafe_background
			)

