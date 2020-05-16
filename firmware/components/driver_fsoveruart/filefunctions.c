#include <esp_err.h>
#include <esp_log.h>
#include <driver/uart.h>
#include <esp_vfs.h>
#include <dirent.h>

#include <esp_task_wdt.h>

#include "include/filefunctions.h"
#include "include/packetutils.h"

#define TAG "fsoveruart_ff"

const char root[] = {"dflash\ndsdcard"};

/***
 * All functions in this file follow the same flow. Each function gets called when data has arrived.
 * This can be partial data, or the complete packet.
 * data - Pointer to the bytes received.
 * Command - Command ID
 * Size - The complete payload size of the command in bytes
 * Received - How many bytes have already been received
 * Length - The length that has been received in this packet.
 * 
 * When the function returns 0 the next packet received will be appended to the previous received data.
 * When the function returns 1 the program will place the next packet at data[0], all previous received data will be deleted.
 * 
 * 
 ***/

//This function fixes the weird mount points in the badge firmware
void buildfile(char *source, char *target) {
    if(strncmp(source, "/flash", 6) == 0) {
        strcpy(target, "/_#!#_spiflash");
        strcat(target, &source[6]);
    } else if(strncmp(source, "/sdcard", 7) == 0) {
        strcpy(target, "/_#!#_sdcard");
        strcat(target, &source[7]);
    }
}

int getdir(uint8_t *data, uint16_t command, uint32_t size, uint32_t received, uint32_t length) {
    if(received != size) return 0;

    if(length == 0 || length == 1 || length == 2) { //Requesting root
        strcat((char *) data, "\n");  //Append folder name and type
        strcat((char *) data, root); //Append root structure
        uint8_t header[8];
        createMessageHeader(header, command, strlen((char *) data));
        uart_write_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, (const char*) header, 8);
        uart_write_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, (const char*) data, strlen((char *) data));
        return 1;
    }
     //TODO: Fix when folder list exceeds buffer
    char dir_name[length+20];   //Take length of the folder and add the spiflash mountpoint
    buildfile((char *) data, dir_name);
    ////ESP_LOGI(TAG, "%s", dir_name);
    DIR *d;
    struct dirent *dir;
    d = opendir(dir_name);  //Loop through all files/directories
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            strcat((char *) data, "\n");  //Append folder name and type
            strcat((char *) data, dir->d_type == DT_DIR ? "d" : "f");
            strcat((char *) data, dir->d_name);
        }
        closedir(d);    
    } else {
        strcpy((char *) data, "Directory_not_found");  //Cant find directory, request dir doesnt exists
    }
    uint8_t header[8];
    createMessageHeader(header, command, strlen((char *) data));
    uart_write_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, (const char*) header, 8);
    uart_write_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, (const char*) data, strlen((char *) data));

    return 1;
}

int readfile(uint8_t *data, uint16_t command, uint32_t size, uint32_t received, uint32_t length) {
    if(received != size) return 0;

    char dir_name[length+20];   //Take length of the folder and add the spiflash mountpoint
    buildfile((char *) data, dir_name);
    //ESP_LOGI(TAG, "Reading: %s", dir_name);

    FILE *fptr_glb;
    fptr_glb = fopen(dir_name, "r");
    if(fptr_glb) {
        fseek(fptr_glb, 0, SEEK_END);
        uint32_t size_file = ftell(fptr_glb);
        //ESP_LOGI(TAG, "file size: %d", size_file);    
        //Create header with file size
        uint8_t header[8];
        createMessageHeader(header, command, size_file);
        uart_write_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, (const char*) header, 8);
        
        fseek(fptr_glb, 0, SEEK_SET);
    //     ////ESP_LOGI(TAG, "%d", (int) fptr_glb);
        uint32_t read_bytes;
        do {
            read_bytes = fread(data, 1, 128, fptr_glb);
            uart_write_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, (const char*) data, read_bytes);
            //esp_task_wdt_reset();
            vTaskDelay(1);
        } while(read_bytes == 128);
        fclose(fptr_glb);
    } else {
    //     strcpy((char *) data, "Can't open file");
    //     uint8_t header[8];
    //     createMessageHeader(header, command, strlen((char *) data));
    //     uart_write_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, (const char*) header, 8);
    //     uart_write_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, (const char*) data, strlen((char *) data));
    }
    return 1;
}

int writefile(uint8_t *data, uint16_t command, uint32_t size, uint32_t received, uint32_t length) {
    static FILE *fptr = NULL;
    static int failed_open = 0;

    if(fptr == NULL && failed_open == 0) {
        for(int i = 0; i < received; i++) {
            if(data[i] == 0) {
                char dir_name[length+20];   //Take length of the folder and add the spiflash mountpoint
                buildfile((char *) data, dir_name);
                //ESP_LOGI(TAG, "Writing: %s", dir_name);

                fptr = fopen(dir_name, "w");
                if(fptr) {
                    if(received > i) {
                        fwrite(&data[i+1], 1, received-i-1, fptr);
                    }
                } else {
                    //ESP_LOGI(TAG, "Open failed");
                    failed_open = 1;
                }

                if(received == size) {    //Creating an empty file or short. Close the file and send reply
                    failed_open = 0;
                    if(fptr) {
                        sendok(command);
                        fclose(fptr);
                        fptr = NULL;
                    } else {
                        sender(command);
                    }
                    //ESP_LOGI(TAG, "File close, sending reply");
                }
                return 1;
            }
        }
        //ESP_LOGI(TAG, "No filename");
        return 0;   //Found no 0 terminator. File path not received.
    } else if(fptr) {
        fwrite(data, 1, length, fptr);
        if(received == size) {  //Finished receiving
            fclose(fptr);
            fptr = NULL;
            failed_open = 0;
            sendok(command);
        } else {
            vTaskDelay(1);
        }
        return 1;
    } else {
        if(received == size) {
            sender(command);
            fptr = NULL;
            failed_open = 0;
        }
        return 1;
    }
}

int delfile(uint8_t *data, uint16_t command, uint32_t size, uint32_t received, uint32_t length) {
    if(received != size) return 0;
    
    char dir_name[length+20];   //Take length of the folder and add the spiflash mountpoint
    buildfile((char *) data, dir_name);
    //ESP_LOGI(TAG, "Del: %s", dir_name);
    
    if(remove(dir_name) == 0) {
        sendok(command);
    } else {
        sender(command);
    }
    return 1;
}

int cpyfile(uint8_t *data, uint16_t command, uint32_t size, uint32_t received, uint32_t length, uint32_t delete_source) {
    int source_len = strlen((char *) data);
    uint8_t *dest = &data[source_len+1];    
    int dest_len = strlen((char *) dest);
    int isfolder = dest[dest_len-1] == '/';

    int filename_index = 0;

    for(int i = source_len-1; i > 0; i--) {
        if(data[i] == '/') {
            filename_index = i+1;
            break;
        }
    }

    if(filename_index == 0) { //no filename found
        sender(command);
        return 1;
    }

    if(isfolder) {
        //ESP_LOGI(TAG, "ADding filename: %s", (char *) &data[filename_index]);
        strcat((char *) dest, (char *) &data[filename_index]);
        dest_len = strlen((char *) dest);
    }

    char source_file[source_len+30];
    buildfile((char *) data, source_file);
    char dest_file[dest_len+30];
    buildfile((char *) dest, dest_file);

    //ESP_LOGI(TAG, "source: %s", source_file);
    //ESP_LOGI(TAG, "dest: %s", dest_file);

    if(strcmp(source_file, dest_file) == 0) return 0;   //If dest and source are the same return error

    FILE *source, *target;
    source = fopen(source_file, "r");
    target = fopen(dest_file, "w");

    if(source && target) {
        int read_bytes;
        uint8_t buf[64];
        do {
            read_bytes = fread(buf, 1, 64, source);
            fwrite(buf, 1, read_bytes, target);
        } while(read_bytes > 0);
        fclose(source);
        fclose(target);
        if(delete_source) remove(source_file);
        return 1;
    } else {
        return 0;
    }

}

int duplfile(uint8_t *data, uint16_t command, uint32_t size, uint32_t received, uint32_t length) {
    if(received != size) return 0;
   
    if(cpyfile(data, command, size, received, length, 0)) {       
        sendok(command);
    } else {
        sender(command);
    }

    return 1;
}

int mvfile(uint8_t *data, uint16_t command, uint32_t size, uint32_t received, uint32_t length) {
    if(received != size) return 0;
   
    if(cpyfile(data, command, size, received, length, 1)) {       
        sendok(command);
    } else {
        sender(command);
    }

    return 1;
}

int makedir(uint8_t *data, uint16_t command, uint32_t size, uint32_t received, uint32_t length) {
    if(received != size) return 0;

    char dir_name[length+20];   //Take length of the folder and add the spiflash mountpoint
    buildfile((char *) data, dir_name);
    //ESP_LOGI(TAG, "mkdir: %s", dir_name);

    if(mkdir(dir_name, 0777) == 0) {
        sendok(command);
    } else {
        sender(command);
    }
    return 1;

}