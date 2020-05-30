#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <esp_err.h>
#include <esp_log.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_vfs.h>
#include <dirent.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "include/driver_fsoveruart.h"
#include "include/filefunctions.h"
#include "include/packetutils.h"
#include "include/specialfunctions.h"

#ifdef CONFIG_DRIVER_FSOVERUART_ENABLE

#define min(a,b) ((a)<(b)?(a):(b))

void fsoveruartTask(void *pvParameter);
void vTimeoutFunction( TimerHandle_t xTimer );

#define TAG "fsoveruart"

uart_config_t uart_config = {
    .baud_rate = CONFIG_DRIVER_FSOVERUART_UART_BAUD,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_RTS,
    .rx_flow_ctrl_thresh = 32,
    };

QueueHandle_t uart_queue;
TimerHandle_t timeout;
uint8_t command_in[1024];


//Function lookup tables

int (*specialfunction[])(uint8_t *data, uint16_t command, uint32_t size, uint32_t received, uint32_t length) = {execfile, heartbeat};
int specialfunction_size = 2;
                         //                                                                                  4096    4097      4098       4099     4100      4101    4102
int (*filefunction[])(uint8_t *data, uint16_t command, uint32_t size, uint32_t received, uint32_t length) = {getdir, readfile, writefile, delfile, duplfile, mvfile, makedir};
int filefunction_size = 7;

void handleFSCommand(uint8_t *data, uint16_t command, uint32_t size, uint32_t received, uint32_t length) {
    static uint32_t write_pos;
    if(received == length) { //First packet
        write_pos = 0;
        //ESP_LOGI(TAG, "clear");
    }
    
    if(command && length > 0) {
        memcpy(&command_in[write_pos], data, length);
        write_pos += length;
    }

    int return_val = 0;
    if(command < 4096) {
        if(command < specialfunction_size) {
            return_val = specialfunction[command](data, command, size, received, length);
        }
    } else if(command < 8192) {
        if((command-4096) < filefunction_size) {
            return_val = filefunction[command-4096](data, command, size, received, length);
        }
    }

    if(return_val) {    //Function has indicated that payload should write at start of buffer.
        write_pos = 0;
    }
}

int receiving = 0;

void vTimeoutFunction( TimerHandle_t xTimer ) {
    ESP_LOGI(TAG, "Saw no message for 1s assuming task crashed. Resetting...");
    receiving = 0;
    sendto(1);
}

void fsoveruartTask(void *pvParameter) {
    uart_event_t event;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    int receiving = 0;
    uint16_t command = 0;
    uint32_t size = 0;
    uint32_t recv = 0;
    uint16_t verif = 0;
    

    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(uart_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            bzero(dtmp, RD_BUF_SIZE);
            uint32_t data_buf = 0;
            uart_get_buffered_data_len(CONFIG_DRIVER_FSOVERUART_UART_NUM, &data_buf);
            //ESP_LOGI(TAG, "buf: %d", data_buf);
            if(data_buf > CONFIG_DRIVER_FSOVERUART_BUFFER_SIZE/4) {
                gpio_pad_select_gpio(CONFIG_DRIVER_FSOVERUART_UART_CTS);
                gpio_set_direction(CONFIG_DRIVER_FSOVERUART_UART_CTS, GPIO_MODE_OUTPUT);
                gpio_set_level(CONFIG_DRIVER_FSOVERUART_UART_CTS, 1);
            } else {
                uart_set_pin(CONFIG_DRIVER_FSOVERUART_UART_NUM, CONFIG_DRIVER_FSOVERUART_UART_TX, CONFIG_DRIVER_FSOVERUART_UART_RX, CONFIG_DRIVER_FSOVERUART_UART_CTS, -1); //Change pins
            }
            uint32_t bytesread = 0;
            uint32_t bytestoread;
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    //ESP_LOGI(TAG, "siz: %d", event.size);                    
                    while(bytesread != data_buf) {
                        if(!receiving) {
                            if((data_buf-bytesread) < 8) break; //Break while loop if non complete header is inside 
                            uart_read_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, dtmp, 8, portMAX_DELAY);
                            bytesread += 8;                        
                            command = *((uint16_t *) &dtmp[0]);
                            size = *((uint32_t *) &dtmp[2]);
                            verif = *((uint16_t *) &dtmp[6]);
                            ESP_LOGI(TAG, "new packet: %d %d %d %d", command, size, verif, event.size-8);
                            if(verif == 0xADDE) {
                                receiving = 1;
                                recv = 0;
                            } else {
                                receiving = 0;
                                uart_flush_input(CONFIG_DRIVER_FSOVERUART_UART_NUM);
                                xQueueReset(uart_queue);
                                //Received wrong command, flushing uart queue
                                sendte(1);
                                return;
                            }
                        } else {
                            xTimerStop(timeout, 1);
                            bytestoread = min(min(data_buf, size), RD_BUF_SIZE);
                            recv = recv + bytestoread;
                            uart_read_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, dtmp, bytestoread, portMAX_DELAY);
                            bytesread += bytestoread;
                            handleFSCommand(dtmp, command, size, recv, bytestoread);
                            if(recv == size) {
                                receiving = 0;
                            }
                        }
                    }                
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(CONFIG_DRIVER_FSOVERUART_UART_NUM);
                    xQueueReset(uart_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(CONFIG_DRIVER_FSOVERUART_UART_NUM);
                    xQueueReset(uart_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                
                    break;
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
            if(receiving) {
                xTimerStart(timeout, 1);
            } else {
                xTimerStop(timeout, 1);
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

#define UART_EMPTY_THRESH_DEFAULT  (10)
#define UART_FULL_THRESH_DEFAULT  (120)
#define UART_TOUT_THRESH_DEFAULT   (10)
#define UART_CLKDIV_FRAG_BIT_WIDTH  (3)
#define UART_TOUT_REF_FACTOR_DEFAULT (UART_CLK_FREQ/(REF_CLK_FREQ<<UART_CLKDIV_FRAG_BIT_WIDTH))
#define UART_TX_IDLE_NUM_DEFAULT   (0)
#define UART_PATTERN_DET_QLEN_DEFAULT (10)
#define UART_MIN_WAKEUP_THRESH      (2)

esp_err_t driver_fsoveruart_init(void) { 
   
    uart_param_config(CONFIG_DRIVER_FSOVERUART_UART_NUM, &uart_config);   //Configure the uart hardware
    uart_set_pin(CONFIG_DRIVER_FSOVERUART_UART_NUM, CONFIG_DRIVER_FSOVERUART_UART_TX, CONFIG_DRIVER_FSOVERUART_UART_RX, CONFIG_DRIVER_FSOVERUART_UART_CTS, -1); //Change pins
    uart_driver_install(CONFIG_DRIVER_FSOVERUART_UART_NUM, CONFIG_DRIVER_FSOVERUART_BUFFER_SIZE, CONFIG_DRIVER_FSOVERUART_BUFFER_SIZE, 20, &uart_queue, 0); //Install driver

    uart_intr_config_t uart_intr = {
        .intr_enable_mask = UART_RXFIFO_FULL_INT_ENA_M
                            | UART_RXFIFO_TOUT_INT_ENA_M
                            | UART_FRM_ERR_INT_ENA_M
                            | UART_RXFIFO_OVF_INT_ENA_M
                            | UART_BRK_DET_INT_ENA_M
                            | UART_PARITY_ERR_INT_ENA_M,
        .rxfifo_full_thresh = 64,
        .rx_timeout_thresh = UART_TOUT_THRESH_DEFAULT,
        .txfifo_empty_intr_thresh = UART_EMPTY_THRESH_DEFAULT
    };
    uart_intr_config(CONFIG_DRIVER_FSOVERUART_UART_NUM, &uart_intr);

    ESP_LOGI(TAG, "fs over uart registered.");
    xTaskCreatePinnedToCore(fsoveruartTask, "fsoveruart", 16000, NULL, 100, NULL, 1);
    timeout = xTimerCreate("FSoverUART_timeout", 100, false, 0, vTimeoutFunction);
    return ESP_OK;
} 

#else
esp_err_t driver_fsoveruart_init(void) { return ESP_OK; } //Dummy function.
#endif