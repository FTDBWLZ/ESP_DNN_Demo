idf_component_register(
        SRC_DIRS
            .
            Adafruit-GFX-Library

        INCLUDE_DIRS
            include
            Adafruit-GFX-Library/Fonts
            Adafruit-GFX-Library

        REQUIRES
            esp_driver_spi
            esp_driver_gpio
            esp_partition

        PRIV_REQUIRES
            spi_flash
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__AVR_ATtiny85__")
