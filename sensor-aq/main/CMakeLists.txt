list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../shared-components")

idf_component_register(SRCS "main.cpp" 
                            "sps30/sps30.c"
                            "sps30/sensirion_shdlc.c"
                            "sps30/sensirion_uart_implementation.c"
                            "../../shared-components/core/src/radio.c"
                            "../../shared-components/core/src/espio.c"
                            "../../shared-components/core/src/espuart.c"
                    INCLUDE_DIRS "."
                            "../../shared-components/core/include"
                    PRIV_REQUIRES nvs_flash driver ieee802154)
