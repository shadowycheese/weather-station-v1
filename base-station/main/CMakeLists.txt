set(LV_DEMO_DIR ../managed_components/lvgl__lvgl/demos)
file(GLOB_RECURSE LV_DEMOS_SOURCES ${LV_DEMO_DIR}/*.c)

idf_component_register(
    SRCS 
        ${LV_DEMOS_SOURCES}
        main.cpp 
        ui/display.cpp
        ui/edt.cpp
        io/sensor.cpp
        ../../shared-components/core/src/espio.c
        ../../shared-components/core/src/espspi.c
    INCLUDE_DIRS 
        ../../shared-components/core/include
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/ui
        ${LV_DEMO_DIR})

idf_component_get_property(LVGL_LIB lvgl__lvgl COMPONENT_LIB)
target_compile_options(
    ${LVGL_LIB} 
    PRIVATE
        -DLV_LVGL_H_INCLUDE_SIMPLE
        -DLV_USE_DEMO_MUSIC
)
