/* LoRa radio driver, based on the Arduino library by Sandeep Mistry
 * https://github.com/sandeepmistry/arduino-LoRa/blob/master/LICENSE
 * 
 * MIT License
 * 
 * Copyright (c) 2016 Sandeep Mistry
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#include <sdkconfig.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include <driver/spi_master.h>
#include <freertos/task.h>

#include "include/driver_lora.h"

#ifdef CONFIG_DRIVER_LORA_ENABLE

#define TAG "lora"

xSemaphoreHandle driver_lora_mux           = NULL; // Mutex for accessing the handler
xSemaphoreHandle driver_lora_intr_trigger  = NULL; // Semaphore to trigger LoRa interrupt handling
static spi_device_handle_t spi_device      = NULL; // SPI device handle for accessing the LoRa radio
static bool __implicit                     = 0;    // LoRa header mode
static long __frequency                    = 0;    // LoRa frequency
driver_lora_intr_t driver_lora_handler     = NULL; // Interrupt handler
void*              driver_lora_handler_arg = NULL; // Argument passed to interrupt handler


/* SPI communication */

esp_err_t driver_lora_write_reg(uint8_t reg, uint8_t val)
{
	uint8_t out[2] = { 0x80 | reg, val };
	uint8_t in[2];

	spi_transaction_t t = {
		.flags = 0,
		.length = 8 * 2,
		.tx_buffer = out,
		.rx_buffer = in  
	};

	return spi_device_transmit(spi_device, &t);
}

esp_err_t driver_lora_read_reg(uint8_t reg, uint8_t* val)
{
	if (!val) return ESP_FAIL;
	uint8_t out[2] = { reg, 0xff };
	uint8_t in[2];
	spi_transaction_t t = {
		.flags = 0,
		.length = 8 * sizeof(out),
		.tx_buffer = out,
		.rx_buffer = in
	};
	esp_err_t res = spi_device_transmit(spi_device, &t);
	if (res != ESP_OK) return res;
	*val = in[1];
	return ESP_OK;
}

/* Basic device control */

esp_err_t driver_lora_reset(void) {
	#if CONFIG_PIN_NUM_LORA_RST >= 0
	esp_err_t res = gpio_set_level(CONFIG_PIN_NUM_LORA_RST, false);
	if (res != ESP_OK) return res;
	ets_delay_us(200000);
	res = gpio_set_level(CONFIG_PIN_NUM_LORA_RST, true);
	if (res != ESP_OK) return res;
	#endif
	ets_delay_us(200000);
	return ESP_OK;
}

/* Header mode */

esp_err_t driver_lora_explicit_header_mode(void)
{
	uint8_t value;
	esp_err_t res = driver_lora_read_reg(REG_MODEM_CONFIG_1, &value);
	if (res != ESP_OK) return res;
	res = driver_lora_write_reg(REG_MODEM_CONFIG_1, value & 0xfe);
	if (res != ESP_OK) return res;
	__implicit = 0;
	return ESP_OK;
}

esp_err_t driver_lora_implicit_header_mode(uint8_t size)
{
	uint8_t value;
	esp_err_t res = driver_lora_read_reg(REG_MODEM_CONFIG_1, &value);
	if (res != ESP_OK) return res;
	res = driver_lora_write_reg(REG_MODEM_CONFIG_1, value | 0x01);
	if (res != ESP_OK) return res;
	res = driver_lora_write_reg(REG_PAYLOAD_LENGTH, size);
	if (res != ESP_OK) return res;
	__implicit = 1;
	return ESP_OK;
}

/* Radio mode */

esp_err_t driver_lora_idle(void)
{
   return driver_lora_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
}

esp_err_t driver_lora_sleep(void)
{ 
   return driver_lora_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
}

esp_err_t driver_lora_receive(void)
{
   return driver_lora_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
}

/* Radio settings */

esp_err_t driver_lora_set_tx_power(uint8_t level)
{
	// RF9x module uses PA_BOOST pin
	if (level < 2) {
		level = 2;
	} else if (level > 17) {
		level = 17;
	}
	return driver_lora_write_reg(REG_PA_CONFIG, PA_BOOST | (level - 2));
}

esp_err_t driver_lora_set_frequency(long frequency)
{
	uint64_t frf = ((uint64_t)frequency << 19) / 32000000;
	esp_err_t res = driver_lora_write_reg(REG_FRF_MSB, (uint8_t)(frf >> 16));
	if (res != ESP_OK) return res;
	res = driver_lora_write_reg(REG_FRF_MID, (uint8_t)(frf >> 8));
	if (res != ESP_OK) return res;
	res = driver_lora_write_reg(REG_FRF_LSB, (uint8_t)(frf >> 0));
	if (res != ESP_OK) return res;
	__frequency = frequency;
	return ESP_OK;
}

esp_err_t driver_lora_set_spreading_factor(uint8_t sf)
{
	if (sf < 6) {
		sf = 6;
	} else if (sf > 12) {
		sf = 12;
	}
	
	esp_err_t res;

	if (sf == 6) {
		res = driver_lora_write_reg(REG_DETECTION_OPTIMIZE, 0xc5);
		if (res != ESP_OK) return res;
		res = driver_lora_write_reg(REG_DETECTION_THRESHOLD, 0x0c);
		if (res != ESP_OK) return res;
	} else {
		res = driver_lora_write_reg(REG_DETECTION_OPTIMIZE, 0xc3);
		if (res != ESP_OK) return res;
		res = driver_lora_write_reg(REG_DETECTION_THRESHOLD, 0x0a);
		if (res != ESP_OK) return res;
	}

	uint8_t value;
	res = driver_lora_read_reg(REG_MODEM_CONFIG_2, &value);
	if (res != ESP_OK) return res;
	
	return driver_lora_write_reg(REG_MODEM_CONFIG_2, (value & 0x0f) | ((sf << 4) & 0xf0));
}

esp_err_t driver_lora_set_bandwidth(long sbw)
{
	uint8_t bw;

	if (sbw <= 7.8E3)        bw = 0;
	else if (sbw <= 10.4E3)  bw = 1;
	else if (sbw <= 15.6E3)  bw = 2;
	else if (sbw <= 20.8E3)  bw = 3;
	else if (sbw <= 31.25E3) bw = 4;
	else if (sbw <= 41.7E3)  bw = 5;
	else if (sbw <= 62.5E3)  bw = 6;
	else if (sbw <= 125E3)   bw = 7;
	else if (sbw <= 250E3)   bw = 8;
	else                     bw = 9;
	
	uint8_t value;
	esp_err_t res = driver_lora_read_reg(REG_MODEM_CONFIG_1, &value);
	if (res != ESP_OK) return res;
	return driver_lora_write_reg(REG_MODEM_CONFIG_1, (value & 0x0f) | (bw << 4));
}

esp_err_t driver_lora_set_coding_rate(uint8_t denominator)
{
	if (denominator < 5) {
		denominator = 5;
	} else if (denominator > 8) {
		denominator = 8;
	}
	uint8_t cr = denominator - 4;
	uint8_t value;
	esp_err_t res = driver_lora_read_reg(REG_MODEM_CONFIG_1, &value);
	if (res != ESP_OK) return res;
	return driver_lora_write_reg(REG_MODEM_CONFIG_1, (value & 0xf1) | (cr << 1));
}

esp_err_t driver_lora_set_preamble_length(long length)
{
	esp_err_t res = driver_lora_write_reg(REG_PREAMBLE_MSB, (uint8_t)(length >> 8));
	if (res != ESP_OK) return res;
	return driver_lora_write_reg(REG_PREAMBLE_LSB, (uint8_t)(length >> 0));
}

esp_err_t driver_lora_set_sync_word(uint8_t sw)
{
	return driver_lora_write_reg(REG_SYNC_WORD, sw);
}

esp_err_t driver_lora_enable_crc(void)
{
	uint8_t value;
	esp_err_t res = driver_lora_read_reg(REG_MODEM_CONFIG_2, &value);
	if (res != ESP_OK) return res;
	return driver_lora_write_reg(REG_MODEM_CONFIG_2, value | 0x04);
}

esp_err_t driver_lora_disable_crc(void)
{
	uint8_t value;
	esp_err_t res = driver_lora_read_reg(REG_MODEM_CONFIG_2, &value);
	if (res != ESP_OK) return res;
	return driver_lora_write_reg(REG_MODEM_CONFIG_2, value & 0xfb);
}

/* Packet transmit */

esp_err_t driver_lora_send_packet(uint8_t *buf, uint8_t size)
{
	esp_err_t res = driver_lora_idle();
	if (res != ESP_OK) return res;
	res = driver_lora_write_reg(REG_FIFO_ADDR_PTR, 0);
	if (res != ESP_OK) return res;

	for (uint8_t i = 0; i<size; i++) {
		res = driver_lora_write_reg(REG_FIFO, *buf++);
		if (res != ESP_OK) return res;
	}

	res = driver_lora_write_reg(REG_PAYLOAD_LENGTH, size);
	if (res != ESP_OK) return res;
	
	res = driver_lora_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);
	if (res != ESP_OK) return res;
	
	uint16_t timeout = 10000;
	
	while (1) {
		uint8_t value;
		esp_err_t res = driver_lora_read_reg(REG_IRQ_FLAGS, &value);
		if (res != ESP_OK) return res;
		if ((value & IRQ_TX_DONE_MASK) == 0) break;
		//printf("(Waiting: 0x%02x)\n", value);
		vTaskDelay(2 / portTICK_PERIOD_MS);
		timeout--;
		if (timeout < 1) break;
	}
	
	if (!timeout) printf("TIMEOUT!!!!!\n");

	return driver_lora_write_reg(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
}

/* Packet receive */

esp_err_t driver_lora_receive_packet(uint8_t *buf, uint8_t bufferSize, uint8_t* len)
{
	*len = 0;
	uint8_t irq;
	esp_err_t res = driver_lora_read_reg(REG_IRQ_FLAGS, &irq);
	if (res != ESP_OK) return res;
	res = driver_lora_write_reg(REG_IRQ_FLAGS, irq);
	if (res != ESP_OK) return res;
	
	if((irq & IRQ_RX_DONE_MASK) == 0)    return ESP_FAIL;
	if(irq & IRQ_PAYLOAD_CRC_ERROR_MASK) return ESP_FAIL;
	
	if (__implicit) {
		res = driver_lora_read_reg(REG_PAYLOAD_LENGTH, len);
		if (res != ESP_OK) return res;
	} else {
		res = driver_lora_read_reg(REG_RX_NB_BYTES, len);
		if (res != ESP_OK) return res;
	}
	
	res = driver_lora_idle();   
	if (res != ESP_OK) return res;
	
	uint8_t fifo_rx_current_addr;
	res = driver_lora_read_reg(REG_FIFO_RX_CURRENT_ADDR, &fifo_rx_current_addr);
	if (res != ESP_OK) return res;
	res = driver_lora_write_reg(REG_FIFO_ADDR_PTR, fifo_rx_current_addr);
	if (res != ESP_OK) return res;
	
	if (*len > bufferSize) *len = bufferSize;
	
	for (uint8_t i = 0; i < *len; i++) {
		res = driver_lora_read_reg(REG_FIFO, &buf[i]);
		if (res != ESP_OK) return res;
	}

	return ESP_OK;
}

esp_err_t driver_lora_received(bool* status)
{
	uint8_t irq;
	esp_err_t res = driver_lora_read_reg(REG_IRQ_FLAGS, &irq);
	if (res != ESP_OK) return res;
	*status = (irq & IRQ_RX_DONE_MASK);
	return ESP_OK;
}

esp_err_t driver_lora_packet_rssi(int* rssi)
{
	uint8_t rssiValue;
	esp_err_t res = driver_lora_read_reg(REG_PKT_RSSI_VALUE, &rssiValue);
	if (res != ESP_OK) return res;
	*rssi = (rssiValue - (__frequency < 868E6 ? 164 : 157));
	return ESP_OK;
}

esp_err_t driver_lora_packet_snr(float* snr)
{
	uint8_t snrValue;
	esp_err_t res = driver_lora_read_reg(REG_PKT_SNR_VALUE, &snrValue);
	if (res != ESP_OK) return res;
	*snr = ((int8_t)snrValue) * 0.25;
	return ESP_OK;
}

/* Debugging */

esp_err_t driver_lora_dump_registers(void)
{
	int i;
	//printf("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
	for(i=0; i<0x40; i++) {
		uint8_t val;
		esp_err_t res = driver_lora_read_reg(i, &val);
		if (res != ESP_OK) return res;
		printf("%02X ", val);
		//if((i & 0x0f) == 0x0f) printf("\n");
	}
	printf("\n");
	return ESP_OK;
}

/* Interrupt handling */

void driver_lora_intr_task(void *arg)
{
	
}

void driver_lora_set_interrupt_handler(driver_lora_intr_t handler, void *arg)
{
	if (driver_lora_mux) xSemaphoreTake(driver_lora_mux, portMAX_DELAY);
	driver_lora_handler     = handler;
	driver_lora_handler_arg = arg;
	if (driver_lora_mux) xSemaphoreGive(driver_lora_mux);
}

void driver_lora_intr_handler(void *arg)
{ /* in interrupt handler */
	if (gpio_get_level(CONFIG_PIN_NUM_LORA_INT) == 0) {
		xSemaphoreGiveFromISR(driver_lora_intr_trigger, NULL);
	}
}

/* Driver initialisation */

esp_err_t driver_lora_init(void)
{
	static bool driver_lora_init_done = false;
	if (driver_lora_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	esp_err_t res;
	
	//Initialize reset GPIO pin
	#if CONFIG_PIN_NUM_LORA_RST >= 0
	res = gpio_set_direction(CONFIG_PIN_NUM_LORA_RST, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	#endif
	
	res = gpio_set_direction(CONFIG_PIN_NUM_LORA_INT, GPIO_MODE_INPUT);
	if (res != ESP_OK) return res;
	
	//Initialize SPI device
	static const spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 9000000,
		.mode           = 0,
		.spics_io_num   = CONFIG_PIN_NUM_LORA_CS,
		.queue_size     = 1,
		.flags          = 0,
		.pre_cb         = 0
	};
	res = spi_bus_add_device(VSPI_HOST, &devcfg, &spi_device);
	if (res != ESP_OK) return res;
	
	//Reset the LoRa radio
	res = driver_lora_reset();
	if (res != ESP_OK) return res;
	
	//Check version
	uint8_t version;
	res = driver_lora_read_reg(REG_VERSION, &version);
	if (res != ESP_OK) return res;
	if (version != 0x12) return ESP_FAIL;
	
	//Enter sleep mode
	res = driver_lora_sleep();
	if (res != ESP_OK) return res;
	
	//Initialize some registers
	res = driver_lora_write_reg(REG_FIFO_RX_BASE_ADDR, 0);
	if (res != ESP_OK) return res;
	res = driver_lora_write_reg(REG_FIFO_TX_BASE_ADDR, 0);
	if (res != ESP_OK) return res;
	uint8_t lnaValue;
	res = driver_lora_read_reg(REG_LNA, &lnaValue);
	if (res != ESP_OK) return res;
	res = driver_lora_write_reg(REG_LNA, lnaValue | 0x03);
	if (res != ESP_OK) return res;
	res = driver_lora_write_reg(REG_MODEM_CONFIG_3, 0x04);
	if (res != ESP_OK) return res;
	res = driver_lora_set_tx_power(17);
	if (res != ESP_OK) return res;
	
	driver_lora_explicit_header_mode();
	
	//Create mux
	driver_lora_mux = xSemaphoreCreateMutex();
	if (driver_lora_mux == NULL) return ESP_ERR_NO_MEM;
	
	//Create semaphore
	driver_lora_intr_trigger = xSemaphoreCreateBinary();
	if (driver_lora_intr_trigger == NULL) return ESP_ERR_NO_MEM;
	
	//Assign interrupt handler
	res = gpio_isr_handler_add(CONFIG_PIN_NUM_LORA_INT, driver_lora_intr_handler, NULL);
	if (res != ESP_OK) return res;

	//Create interrupt task
	xTaskCreate(&driver_lora_intr_task, "LoRa interrupt task", 4096, NULL, 10, NULL);
	xSemaphoreGive(driver_lora_intr_trigger);
	
	gpio_config_t io_conf = {
		.intr_type    = GPIO_INTR_ANYEDGE,
		.mode         = GPIO_MODE_INPUT,
		.pin_bit_mask = 1LL << CONFIG_PIN_NUM_LORA_INT,
		.pull_down_en = 0,
		.pull_up_en   = 1,
	};

	res = gpio_config(&io_conf);
	if (res != ESP_OK) return res;
	
	driver_lora_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

#else // CONFIG_DRIVER_VSPI_ENABLE
esp_err_t driver_lora_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
