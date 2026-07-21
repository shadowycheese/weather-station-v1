list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../shared_components")

idf_component_register(SRCS "main.cpp" 
                            "../../shared-components/core/src/radio.c"
                            "../../shared-components/core/src/utils.c"
                            "../../shared-components/core/src/espio.c"
                            "../../shared-components/core/src/espadc.c"
                    INCLUDE_DIRS "."
                            "../../shared-components/core/include"
                    PRIV_REQUIRES nvs_flash esp_adc ieee802154)
