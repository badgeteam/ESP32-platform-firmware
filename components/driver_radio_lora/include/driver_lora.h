#ifndef DRIVER_LORA_H
#define DRIVER_LORA_H

#include <stdint.h>
#include <esp_err.h>

__BEGIN_DECLS

#define REG_FIFO                       0x00
#define REG_OP_MODE                    0x01
#define REG_FRF_MSB                    0x06
#define REG_FRF_MID                    0x07
#define REG_FRF_LSB                    0x08
#define REG_PA_CONFIG                  0x09
#define REG_LNA                        0x0c
#define REG_FIFO_ADDR_PTR              0x0d
#define REG_FIFO_TX_BASE_ADDR          0x0e
#define REG_FIFO_RX_BASE_ADDR          0x0f
#define REG_FIFO_RX_CURRENT_ADDR       0x10
#define REG_IRQ_FLAGS                  0x12
#define REG_RX_NB_BYTES                0x13
#define REG_PKT_SNR_VALUE              0x19
#define REG_PKT_RSSI_VALUE             0x1a
#define REG_MODEM_CONFIG_1             0x1d
#define REG_MODEM_CONFIG_2             0x1e
#define REG_PREAMBLE_MSB               0x20
#define REG_PREAMBLE_LSB               0x21
#define REG_PAYLOAD_LENGTH             0x22
#define REG_MODEM_CONFIG_3             0x26
#define REG_RSSI_WIDEBAND              0x2c
#define REG_DETECTION_OPTIMIZE         0x31
#define REG_DETECTION_THRESHOLD        0x37
#define REG_SYNC_WORD                  0x39
#define REG_DIO_MAPPING_1              0x40
#define REG_VERSION                    0x42

#define MODE_LONG_RANGE_MODE           0x80
#define MODE_SLEEP                     0x00
#define MODE_STDBY                     0x01
#define MODE_TX                        0x03
#define MODE_RX_CONTINUOUS             0x05
#define MODE_RX_SINGLE                 0x06

#define PA_BOOST                       0x80

#define IRQ_TX_DONE_MASK               0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK     0x20
#define IRQ_RX_DONE_MASK               0x40

#define PA_OUTPUT_RFO_PIN              0
#define PA_OUTPUT_PA_BOOST_PIN         1

typedef void (*driver_lora_intr_t)(void*, bool); // Interrupt handler type

extern esp_err_t driver_lora_init(void);
extern esp_err_t driver_lora_explicit_header_mode(void);
extern esp_err_t driver_lora_implicit_header_mode(uint8_t size);
extern esp_err_t driver_lora_idle(void);
extern esp_err_t driver_lora_sleep(void);
extern esp_err_t driver_lora_receive(void);
extern esp_err_t driver_lora_set_tx_power(uint8_t level);
extern esp_err_t driver_lora_set_frequency(long frequency);
extern esp_err_t driver_lora_set_spreading_factor(uint8_t sf);
extern esp_err_t driver_lora_set_bandwidth(long sbw);
extern esp_err_t driver_lora_set_coding_rate(uint8_t denominator);
extern esp_err_t driver_lora_set_preamble_length(long length);
extern esp_err_t driver_lora_set_sync_word(uint8_t sw);
extern esp_err_t driver_lora_enable_crc(void);
extern esp_err_t driver_lora_disable_crc(void);
extern esp_err_t driver_lora_send_packet(uint8_t *buf, uint8_t size);
extern esp_err_t driver_lora_receive_packet(uint8_t *buf, uint8_t bufferSize, uint8_t* len);
extern esp_err_t driver_lora_received(bool* status);
extern esp_err_t driver_lora_packet_rssi(int* rssi);
extern esp_err_t driver_lora_packet_snr(float* snr);
extern esp_err_t driver_lora_dump_registers(void);

__END_DECLS

#endif
