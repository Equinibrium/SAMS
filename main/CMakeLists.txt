idf_component_register(
    SRCS "main.c" "slc_gate.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_lcd
)
