#ifndef FSOB_BACKEND_H
#define FSOB_BACKEND_H

#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "filefunctions.h"
#include "specialfunctions.h"
#include "packetutils.h"


void fsob_init();

void fsob_write_bytes(const char *src, size_t size);

void fsob_reset();

#endif