idf_component_register(
    SRCS 
        main.cpp 
        ui/display.cpp
        ui/edt.cpp
        data/sensor.cpp
        data/db.cpp
        net/network.cpp
        ../../shared-components/core/src/espio.c
        ../../shared-components/core/src/espspi.c
    INCLUDE_DIRS 
        "../../shared-components/core/include"
        "${CMAKE_CURRENT_LIST_DIR}"
        "${CMAKE_CURRENT_LIST_DIR}/ui"      
        "${CMAKE_CURRENT_LIST_DIR}/net"      
        "${CMAKE_CURRENT_LIST_DIR}/data"      
        "../components/lvgl_wrapper"         
        "../components/bsp_extra/include"          
    PRIV_REQUIRES 
        bsp_extra
        esp_wifi 
        esp_netif 
        esp_event 
        nvs_flash 
        lwip
        lvgl__lvgl
        waveshare__esp32_p4_platform
        waveshare__esp_lcd_ili9881c
)

idf_component_get_property(LVGL_LIB lvgl__lvgl COMPONENT_LIB)
target_compile_options(
    ${LVGL_LIB} 
    PRIVATE
        -DLV_LVGL_H_INCLUDE_SIMPLE

)
