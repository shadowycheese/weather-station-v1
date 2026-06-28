list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../shared_components")

idf_component_register(SRCS "main.cpp" 
                            "../../shared_components/models/src/models.c"
                    INCLUDE_DIRS "."
                            "../../shared_components/models/include"
                    PRIV_REQUIRES nvs_flash ieee802154)
