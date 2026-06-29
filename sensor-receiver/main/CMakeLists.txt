list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../shared-components")

idf_component_register(SRCS "main.cpp" 
                            "../../shared-components/core/src/radio.c"
                            "../../shared-components/core/src/espspi.c"
                    INCLUDE_DIRS "."
                            "../../shared-components/core/include"
                    PRIV_REQUIRES nvs_flash driver ieee802154)
