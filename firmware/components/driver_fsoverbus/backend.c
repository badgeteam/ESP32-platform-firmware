#include "include/fsob_backend.h"
#include "include/driver_fsoverbus.h"
#include <esp_err.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"

#define TAG "FSoverBus"

#define min(a,b) (((a) < (b)) ? (a) : (b))

/*
* Write src array with length size to used bus.
* Implement this function in custom backend when used. Block function call if not possible to write data until possible.
*/
__attribute__((weak)) void fsob_write_bytes(const char *src, size_t size)  {
    abort();
}

#if CONFIG_DRIVER_FSOVERBUS_NOBACKEND_HELPER
//Create ring buffer
RingbufHandle_t buf_handle;
static TaskHandle_t fsob_task_handle = NULL;

int receiving = 0;
uint32_t message_id = 0;

void clearBuffer() {    
    RingbufHandle_t buf_handle_old = buf_handle;
    buf_handle = xRingbufferCreate(CONFIG_DRIVER_FSOVERBUS_NOBACKEND_HELPER_Size, RINGBUF_TYPE_BYTEBUF);
    vRingbufferDelete(buf_handle_old);
}


void fsob_task(void *pvParameters) {
    uint16_t command = 0; //Message command id
    uint32_t size = 0;  //Total message size
    uint32_t recv = 0; //Total bytes received so far
    uint16_t verif = 0; //Verif field
    uint32_t continue_reading;
    for( ;; ) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY );
        continue_reading = 1;
        while(continue_reading) {
            size_t freebuf = xRingbufferGetCurFreeSize(buf_handle);
            if(!receiving) {
                if((CONFIG_DRIVER_FSOVERBUS_NOBACKEND_HELPER_Size-freebuf) >= PACKET_HEADER_SIZE) {
                    fsob_stop_timeout();
                    size_t fetched, fetched_split;
                    uint8_t header_full[PACKET_HEADER_SIZE];
                    
                    //Some extra code is necessary incase of wrap around. This part just checks if the fetch is continues. if not fetch another time

                    uint8_t *header = (uint8_t *) xRingbufferReceiveUpTo(buf_handle, &fetched, 10, PACKET_HEADER_SIZE);
                    if(header == NULL) {
                        return; //This shouldn't happen because we checked if there is data in the buffer
                    }
                    memcpy(header_full, header, fetched);
                    vRingbufferReturnItem(buf_handle, header);
                    if(fetched != PACKET_HEADER_SIZE) {
                        header = (uint8_t *) xRingbufferReceiveUpTo(buf_handle, &fetched_split, 10, PACKET_HEADER_SIZE-fetched);
                        if(header == NULL) {
                            return; //This shouldn't happen because we checked if there is data in the buffer
                        }
                        memcpy(&header_full[fetched], header, PACKET_HEADER_SIZE-fetched);
                        vRingbufferReturnItem(buf_handle, header);
                    }

                    //Check the payload header
                    command = *((uint16_t *) &header_full[0]);
                    size = *((uint32_t *) &header_full[2]);
                    verif = *((uint16_t *) &header_full[6]);
                    message_id = *((uint32_t *) &header_full[8]);
                    ESP_LOGI(TAG, "new packet: %d %d %d %d", command, size, verif, message_id);
                    if(verif == 0xADDE) {
                        receiving = 1;
                        fsob_start_timeout();
                        recv = 0;
                        continue_reading = !(xRingbufferGetCurFreeSize(buf_handle) == CONFIG_DRIVER_FSOVERBUS_NOBACKEND_HELPER_Size);
                    } else {
                        receiving = 0;
                        ESP_LOGI(TAG, "Packet header not correct.");
                        clearBuffer();
                        //Received wrong command, flushing uart queue
                    }

                } else {
                    fsob_start_timeout();
                    continue_reading = 0;
                }
            } else {
                fsob_stop_timeout();    //Stop timeout time since we have received some data
                size_t data_sz;
                size_t max_read = min(RD_BUF_SIZE, size-recv);
                uint8_t *data = (uint8_t *) xRingbufferReceiveUpTo(buf_handle, &data_sz, 0, max_read);
                if(data != NULL) {
                    recv += data_sz;
                    ESP_LOGD(TAG, "len: %d, recv: %d, size: %d", size, recv, data_sz);
                    handleFSCommand(data, command, message_id, size, recv, data_sz);
                    vRingbufferReturnItem(buf_handle, data);
                    if(recv == size) {
                        receiving = 0;
                        ESP_LOGD(TAG, "Packet receive complete");                
                    } else {
                        fsob_start_timeout(); //Re enable the timeout timer since the message is still not fully received
                    }
                }
                continue_reading = !(xRingbufferGetCurFreeSize(buf_handle) == CONFIG_DRIVER_FSOVERBUS_NOBACKEND_HELPER_Size);
            }
        }
    }
}

void fsob_init()  {
    buf_handle = xRingbufferCreate(CONFIG_DRIVER_FSOVERBUS_NOBACKEND_HELPER_Size, RINGBUF_TYPE_BYTEBUF);
    if (buf_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create ring buffer\n");
    }
    xTaskCreatePinnedToCore(fsob_task, "fsoverbus_helper", 16000, NULL, 100, &fsob_task_handle, 0);
}

void fsob_reset()  {
    receiving = 0;
    ESP_LOGD(TAG, "Wiping buffer...");
    clearBuffer();
}

void fsob_receive_bytes(uint8_t *data, size_t len) {
    while(xRingbufferSend(buf_handle, data, len, pdMS_TO_TICKS(1000)) == pdFALSE) {
        vTaskDelay(1);
        xTaskNotifyGive(fsob_task_handle);
    }    
    xTaskNotifyGive(fsob_task_handle);  //Notify the fsoverbus worker thread there is data to be processed
}
#else

__attribute__((weak)) void fsob_init()  {

}

__attribute__((weak)) void fsob_reset()  {

}

void fsob_receive_bytes(uint8_t *data, size_t len) {
    abort();
}
#endif