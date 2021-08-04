#include "include/platform.h"
#include "driver_framebuffer.h"
#include "buses.h"

#define TAG "platform"

esp_err_t isr_init() {
esp_err_t res = gpio_install_isr_service(0);
if (res == ESP_FAIL) {
    ESP_LOGW(TAG, "Failed to install gpio isr service. Ignoring this.");
    res = ESP_OK;
}
if (res != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install gpio isr service: %d", res);
}
return res;
}

bool fbReady = false;

void fatal_error(const char *message) {
printf("A fatal error occurred while initializing the driver for '%s'.\n", message);
if (fbReady) {
#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE
#if defined(CONFIG_DRIVER_EINK_ENABLE) || defined(CONFIG_DRIVER_ILI9341_ENABLE)
    driver_framebuffer_fill(NULL, COLOR_WHITE);
    uint16_t y =
        driver_framebuffer_print(NULL, "Fatal error\n", 0, 0, 1, 1, COLOR_BLACK, &roboto_12pt7b);
    y = driver_framebuffer_print(NULL, "Failure while starting driver.\n", 0, y, 1, 1, COLOR_BLACK,
                                &roboto_12pt7b);
    y = driver_framebuffer_print(NULL, message, 0, y, 1, 1, COLOR_BLACK, &roboto_12pt7b);
    y = driver_framebuffer_print(NULL, "\n\nRestarting in 10 seconds...\n", 0, y, 1, 1, COLOR_BLACK,
                                &roboto_12pt7b);
    driver_framebuffer_flush(0);
#endif
#if defined(CONFIG_DRIVER_SSD1306_ENABLE) || defined(CONFIG_DRIVER_ERC12846_ENABLE)
    driver_framebuffer_fill(NULL, COLOR_BLACK);
    uint16_t y =
        driver_framebuffer_print(NULL, "Fatal error\n", 0, 0, 2, 2, COLOR_WHITE, &ipane7x5);
    y = driver_framebuffer_print(NULL, message, 0, y + 5, 1, 1, COLOR_WHITE, &ipane7x5);
    driver_framebuffer_flush(0);
#endif
#endif
}
restart();
}

void platform_init()
{
    if (isr_init()    != ESP_OK) restart();
    if (start_buses() != ESP_OK) restart();
    
    INIT_DRIVER(pca9555       , "PCA9555"    ) //16-bit I/O expander
    INIT_DRIVER(ice40         , "ICE40"      ) //ICE40 FPGA driver
    INIT_DRIVER(mch2021_stm32 , "STM32"      ) //MCH2021 STM32 driver
    INIT_DRIVER(hub75         , "HUB75"      ) //LED matrix
    INIT_DRIVER(erc12864      , "ERC12864"   ) //128x64 LCD screen
    INIT_DRIVER(ssd1306       , "SSD1306"    ) //128x64 OLED screen
    INIT_DRIVER(eink          , "E-INK"      ) //296x128 e-ink display
    INIT_DRIVER(gxgde0213b1   , "GXGDE0213B1") //E-ink on OHS badge
    INIT_DRIVER(nokia6100     , "NOKIA6100"  ) //Nokia 6100 LCD
    INIT_DRIVER(flipdotter    , "FLIPDOTTER" ) //Otter flipdot display
    INIT_DRIVER(ili9341       , "ILI9341"    ) //LCD display on wrover kit
    INIT_DRIVER(fri3d         , "FRI3D"      ) //LEDs on the Fri3d camp 2018 badge
    INIT_DRIVER(st7735        , "ST7735"     ) //Color display
    INIT_DRIVER(st7789v       , "ST7789V"    ) //Color display
    INIT_DRIVER(ledmatrix     , "LEDMATRIX"  ) //Ledmatrix display
    INIT_DRIVER(framebuffer   , "FRAMEBUFFER") //Framebuffer
    fbReady = true;
    INIT_DRIVER(mpr121        , "MPR121"     ) //I/O expander with touch inputs
    INIT_DRIVER(disobey_samd  , "SAMD"       ) //I/O via the Disobey 2019 SAMD co-processor
    INIT_DRIVER(neopixel      , "NEOPIXEL"   ) //Addressable LEDs
    INIT_DRIVER(apa102        , "APA102"     ) //Addressable LEDs
    INIT_DRIVER(microphone    , "MICROPHONE" ) //Microphone driver
    INIT_DRIVER(mpu6050       , "MPU6050"    ) //Accelerometer driver
    INIT_DRIVER(sdcard        , "SDCARD"     ) //SD card driver
    INIT_DRIVER(lora          , "LORA"       ) //LoRa modem driver
    INIT_DRIVER(am2320        , "AM2320"     ) //AM2320 sensor driver
    fflush(stdout);
    vTaskDelay(100 / portTICK_PERIOD_MS); //Give things time to settle.
}
