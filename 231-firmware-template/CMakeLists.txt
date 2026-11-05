cmake_minimum_required(VERSION 3.15)

set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

include(pico_sdk_import.cmake)

project(231_firmware_template)

pico_sdk_init()

add_compile_options(-Wall)

# libs лежит вне папки проекта: второй аргумент задаёт каталог сборки
add_subdirectory(../libs ${CMAKE_BINARY_DIR}/libs)

# выключатель профилирования: 0 — вызовов секундомеров в образе нет
target_compile_definitions(profiling PUBLIC
    PROFILING_ENABLED=1
)

# =====================================================================
# platform — единственное место в проекте с SDK
# =====================================================================
add_library(platform STATIC
    platform/platform.c
)
# PUBLIC — папка проекта: через неё logic находит firmware.h,
# а main — любой свой заголовок по короткому пути
target_include_directories(platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
# pico_stdlib строго PRIVATE: его заголовки не утекают ни в logic, ни в main
target_link_libraries(platform PRIVATE
    pico_stdlib
)

# =====================================================================
# logic — задачи и api, БЕЗ SDK
# =====================================================================
add_library(logic STATIC
    logic/api/api.c
    logic/api/api-callbacks.c
    logic/led-task/led-task.c
    logic/button-task/button-task.c
    logic/pi-task/pi-task.c
)
target_include_directories(logic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/logic
)
# ВАЖНО: сюда нельзя добавлять pico_stdlib, hardware_* —
# проверка 2.3.7 смотрит, что SDK в logic не подключён вовсе
target_link_libraries(logic
    PUBLIC  stdio_text_protocol
    PRIVATE platform
            profiling
            logging
            systime
            device
)

# =====================================================================
# executable
# =====================================================================
add_executable(${PROJECT_NAME}
    main.c
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    platform
    logic
    pico_stdlib       # нужен ради boot-кода SDK до main()
    profiling
    logging
    device
    systime
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})