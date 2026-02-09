
file(GLOB_RECURSE CPP_SRC FOLLOW_SYMLINKS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" )
file(GLOB_RECURSE C_SRC FOLLOW_SYMLINKS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c")

message(CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
message(CPP_SRC="${CPP_SRC}")
# message(C_SRC="${C_SRC}")

idf_component_register(
    SRCS 
        ${C_SRC}
        ${CPP_SRC}

    PRIV_REQUIRES 
        spi_flash 
        esp_adc
        driver
        esp_event
        esp_timer
        esp_wifi
        spiffs
        esp_http_server
        app_update
        nvs_flash

    INCLUDE_DIRS 
        "."
        "params"
        "../build"

         
    EMBED_FILES ${FAVICON_ICO_FILE} ${INDEX_HTML_FILE} ${MAIN_JS_FILE} ${STYLES_CSS_FILE}
)


separate_arguments(DEFS_LIST NATIVE_COMMAND ${COMMON_DEFS})
target_compile_definitions(${COMPONENT_LIB} PRIVATE ${DEFS_LIST}
    ${FAVICON_ICO_SYMBOL} ${FAVICON_ICO_NAME}
    ${INDEX_HTML_SYMBOL} ${INDEX_HTML_NAME}
    ${MAIN_JS_SYMBOL} ${MAIN_JS_NAME}
    ${STYLES_CSS_SYMBOL} ${STYLES_CSS_NAME}
)

# get_property(target_defs11 TARGET ${COMPONENT_LIB} PROPERTY COMPILE_DEFINITIONS)
# message("CPP_SRCwwwwwwwwwwwwwwwwwwwccc ${target_defs11}")

# message(CPP_SRCwwwwwwwwwwwwwwwwwww="${FAVICON_ICO_FILE}")
# # message(CPP_SRCwwwwwwwwwwwwwwwwwww="${INDEX_HTML_FILE}")
# # message(CPP_SRCwwwwwwwwwwwwwwwwwww="${MAIN_JS_FILE}")
# # message(CPP_SRCwwwwwwwwwwwwwwwwwww="${STYLES_CSS_FILE}")
# message(CPP_SRCwwwwwwwwwwwwwwwwwww="${COMMON_DEFS}")
# message(CPP_SRCwwwwwwwwwwwwwwwwwww="${DEFS_LIST}")
# # message(CPP_SRCwwwwwwwwwwwwwwwwwww="${FAVICON_ICO_SYMBOL} -- ${FAVICON_ICO_NAME}")
