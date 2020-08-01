#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spi_flash.h"
#include "esp_system.h"

#define MAGIC_OTA           1
#define MAGIC_FACTORY_RESET 2

void logo( void );
void restart( void );
void halt( void );
int get_magic( void );

#endif
