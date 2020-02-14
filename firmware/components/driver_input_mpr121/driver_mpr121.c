#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <driver/gpio.h>
#include <driver_i2c.h>
#include "include/driver_mpr121.h"

#ifdef CONFIG_DRIVER_MPR121_ENABLE

static const char *TAG = "mpr121";

xSemaphoreHandle driver_mpr121_mux =
    NULL;  // mutex for accessing driver_mpr121_state, driver_mpr121_handlers, etc..
xSemaphoreHandle driver_mpr121_intr_trigger =
    NULL;  // semaphore to trigger MPR121 interrupt handling

driver_mpr121_intr_t driver_mpr121_handlers[12] = {NULL, NULL, NULL, NULL, NULL, NULL,
                                                   NULL, NULL, NULL, NULL, NULL, NULL};
void *driver_mpr121_arg[12]                     = {NULL, NULL, NULL, NULL, NULL, NULL,
                               NULL, NULL, NULL, NULL, NULL, NULL};

static inline int driver_mpr121_read_reg(uint8_t reg) {
  uint8_t value;
  esp_err_t res = driver_i2c_read_reg(CONFIG_I2C_ADDR_MPR121, reg, &value, 1);
  if (res != ESP_OK) {
    ESP_LOGE(TAG, "i2c read reg(0x%02x): error %d", reg, res);
    return -1;
  }
  ESP_LOGD(TAG, "i2c read reg(0x%02x): 0x%02x", reg, value);
  return value;
}

static inline esp_err_t driver_mpr121_read_regs(uint8_t reg, uint8_t *data, size_t data_len) {
  esp_err_t res = driver_i2c_read_reg(CONFIG_I2C_ADDR_MPR121, reg, data, data_len);

  if (res != ESP_OK) {
    ESP_LOGE(TAG, "i2c read regs(0x%02x, %d): error %d", reg, data_len, res);
    return res;
  }
  return res;
}

static inline esp_err_t driver_mpr121_write_reg(uint8_t reg, uint8_t value) {
  esp_err_t res = driver_i2c_write_reg(CONFIG_I2C_ADDR_MPR121, reg, value);
  if (res != ESP_OK) {
    ESP_LOGE(TAG, "i2c write reg(0x%02x, 0x%02x): error %d", reg, value, res);
    return res;
  }
  ESP_LOGD(TAG, "i2c write reg(0x%02x, 0x%02x): ok", reg, value);
  return res;
}

bool touch_values[12] = {false, false, false, false, false, false,
                         false, false, false, false, false, false};

void driver_mpr121_intr_task(void *arg) {
  // we cannot use I2C in the interrupt handler, so we
  // create an extra thread for this..

  int old_touch_state = 0;
  int old_gpio_state  = 0;

  while (1) {
    if (xSemaphoreTake(driver_mpr121_intr_trigger, portMAX_DELAY)) {
      int touch_state, gpio_state;
      while (1) {
        touch_state = driver_mpr121_get_interrupt_status_touch();
        gpio_state  = driver_mpr121_get_interrupt_status_gpio();
        if ((touch_state != -1) && (gpio_state != -1))
          break;
        ESP_LOGE(TAG, "failed to read status registers.");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }

      // Touch events
      for (int i = 0; i < 12; i++) {
        if ((touch_state & (1 << i)) != (old_touch_state & (1 << i))) {
          touch_values[i] = (touch_state & (1 << i));
          xSemaphoreTake(driver_mpr121_mux, portMAX_DELAY);
          driver_mpr121_intr_t handler = driver_mpr121_handlers[i];
          void *arg                    = driver_mpr121_arg[i];
          xSemaphoreGive(driver_mpr121_mux);
          if (handler != NULL)
            handler(arg, (touch_state & (1 << i)) != 0);
        }
      }

      // if (old_gpio_state != gpio_state) printf("MPR121 GPIO STATE CHANGED %d => %d\n",
      // old_gpio_state, gpio_state);

      for (int i = 0; i < 8; i++) {  // Only ELE4-ELE11 have GPIO capabilities
        if ((gpio_state & (1 << i)) != (old_gpio_state & (1 << i))) {
          xSemaphoreTake(driver_mpr121_mux, portMAX_DELAY);
          driver_mpr121_intr_t handler = driver_mpr121_handlers[i + 4];
          void *arg                    = driver_mpr121_arg[i + 4];
          xSemaphoreGive(driver_mpr121_mux);
          if (handler != NULL)
            handler(arg, (gpio_state & (1 << i)) != 0);
        }
      }

      // Over-current protection
      if (touch_state & 0x8000) {
        ESP_LOGE(TAG, "over-current detected!");
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // clear OVCF by writing a 1
        esp_err_t res = driver_mpr121_write_reg(0x01, touch_state >> 8);
        if (res != ESP_OK) {
          ESP_LOGE(TAG, "failed to reset over-current.");
        }

        // enable run-mode, set base-line tracking
        res = driver_mpr121_write_reg(0x5e, 0x88);
        if (res != ESP_OK) {
          ESP_LOGE(TAG, "failed to re-enable touch.");
        }
      }

      old_touch_state = touch_state;
      old_gpio_state  = gpio_state;
    }
  }
}

void driver_mpr121_intr_handler(void *arg) { /* in interrupt handler */
  if (gpio_get_level(CONFIG_PIN_NUM_MPR121_INT) == 0) {
    xSemaphoreGiveFromISR(driver_mpr121_intr_trigger, NULL);
  }
}

esp_err_t driver_mpr121_stop() {
  return driver_mpr121_write_reg(MPR121_ECR, 0);
}

esp_err_t driver_mpr121_start(bool baselineTracking) {
  uint8_t lastTouch = 0;
  for (lastTouch = 0; lastTouch < 12; lastTouch++) {
    if (!driver_mpr121_is_touch_input(lastTouch))
      break;
  }

  uint8_t base = 0x80;
  if (!baselineTracking)
    base = 0x40;

  // printf("MPR121 started ECR 0x%02x\n", base | lastTouch);
  return driver_mpr121_write_reg(MPR121_ECR, base | lastTouch);
}

static esp_err_t configure_gpios(void) {
#ifdef CONFIG_MPR121_ELE4_INPUT
  if (driver_mpr121_configure_gpio(4, MPR121_INPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE4_INPUT_PULL_DOWN
  if (driver_mpr121_configure_gpio(4, MPR121_INPUT_PULL_UP) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE4_INPUT_PULL_UP
  if (driver_mpr121_configure_gpio(4, MPR121_INPUT_PULL_DOWN) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE4_OUTPUT
  if (driver_mpr121_configure_gpio(4, MPR121_OUTPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE4_OUTPUT_LOW_ONLY
  if (driver_mpr121_configure_gpio(4, MPR121_OUTPUT_LOW_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE4_OUTPUT_HIGH_ONLY
  if (driver_mpr121_configure_gpio(4, MPR121_OUTPUT_HIGH_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE5_INPUT
  if (driver_mpr121_configure_gpio(5, MPR121_INPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE5_INPUT_PULL_DOWN
  if (driver_mpr121_configure_gpio(5, MPR121_INPUT_PULL_UP) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE5_INPUT_PULL_UP
  if (driver_mpr121_configure_gpio(5, MPR121_INPUT_PULL_DOWN) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE5_OUTPUT
  if (driver_mpr121_configure_gpio(5, MPR121_OUTPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE5_OUTPUT_LOW_ONLY
  if (driver_mpr121_configure_gpio(5, MPR121_OUTPUT_LOW_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE5_OUTPUT_HIGH_ONLY
  if (driver_mpr121_configure_gpio(5, MPR121_OUTPUT_HIGH_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE6_INPUT
  if (driver_mpr121_configure_gpio(6, MPR121_INPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE6_INPUT_PULL_DOWN
  if (driver_mpr121_configure_gpio(6, MPR121_INPUT_PULL_UP) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE6_INPUT_PULL_UP
  if (driver_mpr121_configure_gpio(6, MPR121_INPUT_PULL_DOWN) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE6_OUTPUT
  if (driver_mpr121_configure_gpio(6, MPR121_OUTPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE6_OUTPUT_LOW_ONLY
  if (driver_mpr121_configure_gpio(6, MPR121_OUTPUT_LOW_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE6_OUTPUT_HIGH_ONLY
  if (driver_mpr121_configure_gpio(6, MPR121_OUTPUT_HIGH_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE7_INPUT
  if (driver_mpr121_configure_gpio(7, MPR121_INPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE7_INPUT_PULL_DOWN
  if (driver_mpr121_configure_gpio(7, MPR121_INPUT_PULL_UP) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE7_INPUT_PULL_UP
  if (driver_mpr121_configure_gpio(7, MPR121_INPUT_PULL_DOWN) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE7_OUTPUT
  if (driver_mpr121_configure_gpio(7, MPR121_OUTPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE7_OUTPUT_LOW_ONLY
  if (driver_mpr121_configure_gpio(7, MPR121_OUTPUT_LOW_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE7_OUTPUT_HIGH_ONLY
  if (driver_mpr121_configure_gpio(7, MPR121_OUTPUT_HIGH_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE8_INPUT
  if (driver_mpr121_configure_gpio(8, MPR121_INPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE8_INPUT_PULL_DOWN
  if (driver_mpr121_configure_gpio(8, MPR121_INPUT_PULL_UP) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE8_INPUT_PULL_UP
  if (driver_mpr121_configure_gpio(8, MPR121_INPUT_PULL_DOWN) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE8_OUTPUT
  if (driver_mpr121_configure_gpio(8, MPR121_OUTPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE8_OUTPUT_LOW_ONLY
  if (driver_mpr121_configure_gpio(8, MPR121_OUTPUT_LOW_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE8_OUTPUT_HIGH_ONLY
  if (driver_mpr121_configure_gpio(8, MPR121_OUTPUT_HIGH_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE9_INPUT
  if (driver_mpr121_configure_gpio(9, MPR121_INPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE9_INPUT_PULL_DOWN
  if (driver_mpr121_configure_gpio(9, MPR121_INPUT_PULL_UP) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE9_INPUT_PULL_UP
  if (driver_mpr121_configure_gpio(9, MPR121_INPUT_PULL_DOWN) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE9_OUTPUT
  if (driver_mpr121_configure_gpio(9, MPR121_OUTPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE9_OUTPUT_LOW_ONLY
  if (driver_mpr121_configure_gpio(9, MPR121_OUTPUT_LOW_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE9_OUTPUT_HIGH_ONLY
  if (driver_mpr121_configure_gpio(9, MPR121_OUTPUT_HIGH_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE10_INPUT
  if (driver_mpr121_configure_gpio(10, MPR121_INPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE10_INPUT_PULL_DOWN
  if (driver_mpr121_configure_gpio(10, MPR121_INPUT_PULL_UP) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE10_INPUT_PULL_UP
  if (driver_mpr121_configure_gpio(10, MPR121_INPUT_PULL_DOWN) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE10_OUTPUT
  if (driver_mpr121_configure_gpio(10, MPR121_OUTPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE10_OUTPUT_LOW_ONLY
  if (driver_mpr121_configure_gpio(10, MPR121_OUTPUT_LOW_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE10_OUTPUT_HIGH_ONLY
  if (driver_mpr121_configure_gpio(10, MPR121_OUTPUT_HIGH_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE11_INPUT
  if (driver_mpr121_configure_gpio(11, MPR121_INPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE11_INPUT_PULL_DOWN
  if (driver_mpr121_configure_gpio(11, MPR121_INPUT_PULL_UP) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE11_INPUT_PULL_UP
  if (driver_mpr121_configure_gpio(11, MPR121_INPUT_PULL_DOWN) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE11_OUTPUT
  if (driver_mpr121_configure_gpio(11, MPR121_OUTPUT) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE11_OUTPUT_LOW_ONLY
  if (driver_mpr121_configure_gpio(11, MPR121_OUTPUT_LOW_ONLY) < 0)
    return ESP_FAIL;
#endif
#ifdef CONFIG_MPR121_ELE11_OUTPUT_HIGH_ONLY
  if (driver_mpr121_configure_gpio(11, MPR121_OUTPUT_HIGH_ONLY) < 0)
    return ESP_FAIL;
#endif
  esp_err_t res;
#if defined(CONFIG_MPR121_OUTPUT_DEFAULT_ON_4)
  res = driver_mpr121_set_gpio_level(4, 1);
  if (res != ESP_OK)
    return res;
#endif
#if defined(CONFIG_MPR121_OUTPUT_DEFAULT_ON_5)
  res = driver_mpr121_set_gpio_level(5, 1);
  if (res != ESP_OK)
    return res;
#endif
#if defined(CONFIG_MPR121_OUTPUT_DEFAULT_ON_6)
  res = driver_mpr121_set_gpio_level(6, 1);
  if (res != ESP_OK)
    return res;
#endif
#if defined(CONFIG_MPR121_OUTPUT_DEFAULT_ON_7)
  res = driver_mpr121_set_gpio_level(7, 1);
  if (res != ESP_OK)
    return res;
#endif
#if defined(CONFIG_MPR121_OUTPUT_DEFAULT_ON_8)
  res = driver_mpr121_set_gpio_level(8, 1);
  if (res != ESP_OK)
    return res;
#endif
#if defined(CONFIG_MPR121_OUTPUT_DEFAULT_ON_9)
  res = driver_mpr121_set_gpio_level(9, 1);
  if (res != ESP_OK)
    return res;
#endif
#if defined(CONFIG_MPR121_OUTPUT_DEFAULT_ON_10)
  res = driver_mpr121_set_gpio_level(10, 1);
  if (res != ESP_OK)
    return res;
#endif
#if defined(CONFIG_MPR121_OUTPUT_DEFAULT_ON_11)
  res = driver_mpr121_set_gpio_level(11, 1);
  if (res != ESP_OK)
    return res;
#endif
  return ESP_OK;
}

esp_err_t driver_mpr121_configure(const uint32_t *baseline, uint8_t touch, uint8_t release) {
  esp_err_t res;
  res = driver_mpr121_write_reg(MPR121_SOFTRESET, 0x63);  // Soft-reset
  if (res != ESP_OK)
    return res;
  vTaskDelay(1 / portTICK_PERIOD_MS);

  res = driver_mpr121_write_reg(MPR121_ECR, 0);
  if (res != ESP_OK)
    return res;

  uint8_t value;
  res = driver_mpr121_read_regs(MPR121_CONFIG2, (uint8_t *)&value, 1);
  if (value != 0x24)
    return ESP_FAIL;

  driver_mpr121_stop();
  for (int i = 0; i < 12; i++) {
    if (driver_mpr121_is_touch_input(i)) {
      res = driver_mpr121_write_reg(MPR121_TOUCHTH_0 + 2 * i, touch);  // touch
      if (res != ESP_OK)
        return res;
      res = driver_mpr121_write_reg(MPR121_RELEASETH_0 + 2 * i, release);  // release
      if (res != ESP_OK)
        return res;
      if (baseline != NULL) {
        // printf("MPR121 baseline #%u is now set to %u >> 2 = %u\n", i, baseline[i], baseline[i] >>
        // 2);
        res = driver_mpr121_write_reg(MPR121_BASELINE_0 + i, baseline[i] >> 2);  // baseline
        if (res != ESP_OK)
          return res;
      }
    }
  }
  driver_mpr121_start(baseline == NULL);

  driver_mpr121_write_reg(MPR121_MHDR, 0x01);
  driver_mpr121_write_reg(MPR121_NHDR, 0x01);
  driver_mpr121_write_reg(MPR121_NCLR, 0x0E);
  driver_mpr121_write_reg(MPR121_FDLR, 0x00);

  driver_mpr121_write_reg(MPR121_MHDF, 0x01);
  driver_mpr121_write_reg(MPR121_NHDF, 0x05);
  driver_mpr121_write_reg(MPR121_NCLF, 0x01);
  driver_mpr121_write_reg(MPR121_FDLF, 0x00);

  driver_mpr121_write_reg(MPR121_NHDT, 0x00);
  driver_mpr121_write_reg(MPR121_NCLT, 0x00);
  driver_mpr121_write_reg(MPR121_FDLT, 0x00);

  driver_mpr121_write_reg(MPR121_DEBOUNCE, 0x00);
  driver_mpr121_write_reg(MPR121_CONFIG1, 0x10);  // default, 16µA charge current
  driver_mpr121_write_reg(MPR121_CONFIG2, 0x20);  // 0x5µs encoding, 1ms period

  /*
  //Autoconfig
  res = driver_mpr121_write_reg(MPR121_AUTOCONFIG0, 0x0B);
  if (res != ESP_OK) return res;
  res = driver_mpr121_write_reg(MPR121_UPLIMIT, 200);
  if (res != ESP_OK) return res;
  res = driver_mpr121_write_reg(MPR121_TARGETLIMIT, 180);
  if (res != ESP_OK) return res;
  res = driver_mpr121_write_reg(MPR121_LOWLIMIT, 130);
  if (res != ESP_OK) return res;
  */

  driver_mpr121_start(baseline == NULL);

  configure_gpios();

  ESP_LOGD(TAG, "configure done");
  return ESP_OK;
}

static esp_err_t nvs_baseline_helper(uint8_t idx, uint32_t *value) {
  if (idx > 11) {
    ESP_LOGE(TAG, "NVS baseline index out of range: %d", idx);
    return -1;
  }
  char key[14];
  sprintf(key, "mpr121.base.%d", idx);
  uint16_t v;
  nvs_handle my_handle;
  esp_err_t err = nvs_open("system", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
    return err;
  err = nvs_get_u16(my_handle, key, &v);
  if (err == ESP_OK)
    *value = v;
  nvs_close(my_handle);
  return err;
}

esp_err_t driver_mpr121_init(void) {
  static bool driver_mpr121_init_done = false;
  if (driver_mpr121_init_done)
    return ESP_OK;
  ESP_LOGD(TAG, "init called");
  esp_err_t res = driver_i2c_init();
  if (res != ESP_OK)
    return res;
  driver_mpr121_mux = xSemaphoreCreateMutex();
  if (driver_mpr121_mux == NULL)
    return ESP_ERR_NO_MEM;
  driver_mpr121_intr_trigger = xSemaphoreCreateBinary();
  if (driver_mpr121_intr_trigger == NULL)
    return ESP_ERR_NO_MEM;
  res = gpio_isr_handler_add(CONFIG_PIN_NUM_MPR121_INT, driver_mpr121_intr_handler, NULL);
  if (res != ESP_OK)
    return res;

  gpio_config_t io_conf = {
      .intr_type    = GPIO_INTR_ANYEDGE,
      .mode         = GPIO_MODE_INPUT,
      .pin_bit_mask = 1LL << CONFIG_PIN_NUM_MPR121_INT,
      .pull_down_en = 0,
      .pull_up_en   = 1,
  };

  res = gpio_config(&io_conf);
  if (res != ESP_OK)
    return res;
  xTaskCreate(&driver_mpr121_intr_task, "MPR121 interrupt task", 1024, NULL, 10, NULL);
  xSemaphoreGive(driver_mpr121_intr_trigger);

  uint32_t mpr121_baseline[12];
  bool use_baseline = true;
  for (int i = 0; i < 12; i++) {
    if (nvs_baseline_helper(i, &mpr121_baseline[i]) != ESP_OK) {
      use_baseline = false;
      break;
    }
  }
  esp_err_t err;
  if (use_baseline) {
    // printf("MPR121 is using calibration from NVS!\n");
    err = driver_mpr121_configure(mpr121_baseline, CONFIG_DRIVER_MPR121_THRESHOLD_PRESS,
                                  CONFIG_DRIVER_MPR121_THRESHOLD_RELEASE);
  } else {
    // printf("MPR121 is using automatic configuration!\n");
    err = driver_mpr121_configure(NULL, CONFIG_DRIVER_MPR121_THRESHOLD_PRESS,
                                  CONFIG_DRIVER_MPR121_THRESHOLD_RELEASE);
  }

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "driver_mpr121_configure failed: %d", err);
    return err;
  }

  err = configure_gpios();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "GPIO configuration failed!");
    return err;
  }

  driver_mpr121_init_done = true;
  ESP_LOGD(TAG, "init done");
  return ESP_OK;
}

void driver_mpr121_set_interrupt_handler(uint8_t pin, driver_mpr121_intr_t handler, void *arg) {
  if (driver_mpr121_mux ==
      NULL) {  // allow setting handlers when driver_mpr121 is not initialized yet.
    driver_mpr121_handlers[pin] = handler;
    driver_mpr121_arg[pin]      = arg;
  } else {
    xSemaphoreTake(driver_mpr121_mux, portMAX_DELAY);
    driver_mpr121_handlers[pin] = handler;
    driver_mpr121_arg[pin]      = arg;
    xSemaphoreGive(driver_mpr121_mux);
  }
}

int driver_mpr121_get_interrupt_status_touch(void) {
  uint16_t value;
  esp_err_t res = driver_mpr121_read_regs(0x00, (uint8_t *)&value, 2);
  if (res != ESP_OK)
    return -1;
  return value;
}

int driver_mpr121_get_interrupt_status_gpio(void) {
  uint8_t value;
  esp_err_t res = driver_mpr121_read_regs(0x75, (uint8_t *)&value, 1);
  if (res != ESP_OK)
    return -1;
  return value;
}

esp_err_t driver_mpr121_get_touch_info(struct driver_mpr121_touch_info *info) {
  int value = driver_mpr121_read_reg(0x00);
  if (value == -1)
    return ESP_FAIL;  // need more-specific error?
  info->touch_state = value;

  uint16_t data[24];
  esp_err_t res = driver_mpr121_read_regs(0x04, (uint8_t *)&data, 24);
  if (res != ESP_OK)
    return res;

  uint8_t baseline[12];
  res = driver_mpr121_read_regs(0x1e, baseline, 12);
  if (res != ESP_OK)
    return res;

  uint8_t touch_release[24];
  res = driver_mpr121_read_regs(0x41, touch_release, 24);
  if (res != ESP_OK)
    return res;

  for (int i = 0; i < 12; i++) {
    info->data[i]     = data[i];
    info->baseline[i] = baseline[i];
    info->touch[i]    = touch_release[i * 2 + 0];
    info->release[i]  = touch_release[i * 2 + 1];
  }

  return ESP_OK;
}

int mpr121_gpio_bit_out[8] = {-1, -1, -1, -1, -1, -1, -1, -1};

int driver_mpr121_configure_gpio(int pin, enum driver_mpr121_gpio_config config) {
  if (pin < 4 || pin >= 12)
    return -1;

  pin -= 4;
  int bit_set = 1 << pin;
  int bit_rst = bit_set ^ 0xff;

  mpr121_gpio_bit_out[pin] = -1;

  // set control 0: 0
  int value = driver_mpr121_read_reg(0x73);
  if (value == -1)
    return -1;
  if ((config & 1) == 0) {
    value &= bit_rst;
  } else {
    value |= bit_set;
  }
  esp_err_t res = driver_mpr121_write_reg(0x73, value);
  if (res != ESP_OK)
    return -1;

  // set control 1: 0
  value = driver_mpr121_read_reg(0x74);
  if (value == -1)
    return -1;

  if ((config & 2) == 0) {
    value &= bit_rst;
  } else {
    value |= bit_set;
  }

  res = driver_mpr121_write_reg(0x74, value);
  if (res != ESP_OK)
    return -1;

  // set data: 0 = low
  value = driver_mpr121_read_reg(0x75);
  if (value == -1)
    return -1;

  // always reset data out bit
  value &= bit_rst;

  res = driver_mpr121_write_reg(0x75, value);
  if (res != ESP_OK)
    return -1;

  // set direction: 1 = output
  value = driver_mpr121_read_reg(0x76);
  if (value == -1)
    return -1;

  if ((config & 4) == 0) {
    value &= bit_rst;
  } else {
    value |= bit_set;
  }

  res = driver_mpr121_write_reg(0x76, value);
  if (res != ESP_OK)
    return -1;

  // enable gpio pin: 1 = enable
  value = driver_mpr121_read_reg(0x77);
  if (value == -1)
    return -1;

  if ((config & 8) == 0) {
    value &= bit_rst;
  } else {
    value |= bit_set;
  }

  res = driver_mpr121_write_reg(0x77, value);
  if (res != ESP_OK)
    return -1;
  return 0;
}

int driver_mpr121_get_gpio_level(int pin) {
  if (pin < 4 || pin >= 12)
    return -1;
  pin &= 7;
  // read data from status register
  int value = driver_mpr121_read_reg(pin < 4 ? 0x01 : 0x00);
  if (value == -1)
    return -1;
  return (value >> pin) & 1;
}

int driver_mpr121_set_gpio_level(int pin, int value) {
  if (pin < 4 || pin >= 12)
    return ESP_ERR_INVALID_ARG;

  pin -= 4;
  if (mpr121_gpio_bit_out[pin] == value)
    return ESP_OK;

  mpr121_gpio_bit_out[pin] = -1;
  int bit_set              = 1 << pin;
  if (value == 0) {
    int res = driver_mpr121_write_reg(0x79, bit_set);  // clear bit
    if (res == ESP_OK)
      mpr121_gpio_bit_out[pin] = 0;
    return res;
  } else {
    int res = driver_mpr121_write_reg(0x78, bit_set);  // set bit
    if (res == ESP_OK)
      mpr121_gpio_bit_out[pin] = 1;
    return res;
  }
}

bool driver_mpr121_is_digital_output(int ele) {
#if defined(CONFIG_MPR121_ELE4_OUTPUT) || defined(CONFIG_MPR121_ELE4_OUTPUT_LOW_ONLY) || \
    defined(CONFIG_MPR121_ELE4_OUTPUT_HIGH_ONLY)
  if (ele == 4)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE5_OUTPUT) || defined(CONFIG_MPR121_ELE5_OUTPUT_LOW_ONLY) || \
    defined(CONFIG_MPR121_ELE5_OUTPUT_HIGH_ONLY)
  if (ele == 5)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE6_OUTPUT) || defined(CONFIG_MPR121_ELE6_OUTPUT_LOW_ONLY) || \
    defined(CONFIG_MPR121_ELE6_OUTPUT_HIGH_ONLY)
  if (ele == 6)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE7_OUTPUT) || defined(CONFIG_MPR121_ELE7_OUTPUT_LOW_ONLY) || \
    defined(CONFIG_MPR121_ELE7_OUTPUT_HIGH_ONLY)
  if (ele == 7)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE8_OUTPUT) || defined(CONFIG_MPR121_ELE8_OUTPUT_LOW_ONLY) || \
    defined(CONFIG_MPR121_ELE8_OUTPUT_HIGH_ONLY)
  if (ele == 8)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE9_OUTPUT) || defined(CONFIG_MPR121_ELE9_OUTPUT_LOW_ONLY) || \
    defined(CONFIG_MPR121_ELE9_OUTPUT_HIGH_ONLY)
  if (ele == 9)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE10_OUTPUT) || defined(CONFIG_MPR121_ELE10_OUTPUT_LOW_ONLY) || \
    defined(CONFIG_MPR121_ELE10_OUTPUT_HIGH_ONLY)
  if (ele == 10)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE11_OUTPUT) || defined(CONFIG_MPR121_ELE11_OUTPUT_LOW_ONLY) || \
    defined(CONFIG_MPR121_ELE11_OUTPUT_HIGH_ONLY)
  if (ele == 11)
    return true;
#endif
  return false;
}

bool driver_mpr121_is_digital_input(int ele) {
#if defined(CONFIG_MPR121_ELE4_INPUT) || defined(CONFIG_MPR121_ELE4_INPUT_PULL_DOWN) || \
    defined(CONFIG_MPR121_ELE4_INPUT_PULL_UP)
  if (ele == 4)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE5_INPUT) || defined(CONFIG_MPR121_ELE5_INPUT_PULL_DOWN) || \
    defined(CONFIG_MPR121_ELE5_INPUT_PULL_UP)
  if (ele == 5)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE6_INPUT) || defined(CONFIG_MPR121_ELE6_INPUT_PULL_DOWN) || \
    defined(CONFIG_MPR121_ELE6_INPUT_PULL_UP)
  if (ele == 6)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE7_INPUT) || defined(CONFIG_MPR121_ELE7_INPUT_PULL_DOWN) || \
    defined(CONFIG_MPR121_ELE7_INPUT_PULL_UP)
  if (ele == 7)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE8_INPUT) || defined(CONFIG_MPR121_ELE8_INPUT_PULL_DOWN) || \
    defined(CONFIG_MPR121_ELE8_INPUT_PULL_UP)
  if (ele == 8)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE9_INPUT) || defined(CONFIG_MPR121_ELE9_INPUT_PULL_DOWN) || \
    defined(CONFIG_MPR121_ELE9_INPUT_PULL_UP)
  if (ele == 9)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE10_INPUT) || defined(CONFIG_MPR121_ELE10_INPUT_PULL_DOWN) || \
    defined(CONFIG_MPR121_ELE10_INPUT_PULL_UP)
  if (ele == 10)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE11_INPUT) || defined(CONFIG_MPR121_ELE11_INPUT_PULL_DOWN) || \
    defined(CONFIG_MPR121_ELE11_INPUT_PULL_UP)
  if (ele == 11)
    return true;
#endif
  return false;
}

bool driver_mpr121_is_touch_input(int ele) {
#if defined(CONFIG_MPR121_ELE0_BTN)
  if (ele == 0)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE1_BTN)
  if (ele == 1)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE2_BTN)
  if (ele == 2)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE3_BTN)
  if (ele == 3)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE4_BTN)
  if (ele == 4)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE5_BTN)
  if (ele == 5)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE6_BTN)
  if (ele == 6)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE7_BTN)
  if (ele == 7)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE8_BTN)
  if (ele == 8)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE9_BTN)
  if (ele == 9)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE10_BTN)
  if (ele == 10)
    return true;
#endif
#if defined(CONFIG_MPR121_ELE11_BTN)
  if (ele == 11)
    return true;
#endif
  return false;
}

int driver_mpr121_get_touch_level(int pin) {
  if ((pin < 0) || (pin > 11))
    return -1;
  return touch_values[pin];
}

#else
esp_err_t driver_mpr121_init(void) {
  return ESP_OK;
}  // Dummy function, leave empty.
#endif
