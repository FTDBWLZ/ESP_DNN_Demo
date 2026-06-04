
#define SPI_MISO    -1
#define SPI_MOSI    47
#define SPI_SCLK    21

#define SD_CS       0
#define SD_MOSI     SPI_MOSI
#define SD_MISO     SPI_MISO
#define SD_CLK      SPI_SCLK

#define TFT_MISO    SPI_MISO
#define TFT_MOSI    SPI_MOSI
#define TFT_SCLK    SPI_SCLK
#define TFT_CS      14
#define TFT_DC      41
#define TFT_BK      GPIO_NUM_MAX
#define TFT_RST     GPIO_NUM_MAX

#define TFT_WITDH   240
#define TFT_HEIGHT  320

#define I2C_SDA     4
#define I2C_SCL     5

#define IIS_SCLK    14
#define IIS_LCLK    32
#define IIS_DSIN    -1
#define IIS_DOUT    33

#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    15
#define SIOD_GPIO_NUM    4
#define SIOC_GPIO_NUM    5

#define Y9_GPIO_NUM      16
#define Y8_GPIO_NUM      17
#define Y7_GPIO_NUM      18
#define Y6_GPIO_NUM      12
#define Y5_GPIO_NUM      10
#define Y4_GPIO_NUM      8
#define Y3_GPIO_NUM      9
#define Y2_GPIO_NUM      11
#define VSYNC_GPIO_NUM   6
#define HREF_GPIO_NUM    7
#define PCLK_GPIO_NUM    13
#define XCLK_FREQ        16000000
