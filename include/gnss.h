/******************************
| Project       : MSE Avionics
| Board         : SEQ
| Description   : GNSS header
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#ifndef GNSS_HEADER_FILE
#define GNSS_HEADER_FILE

#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_u-blox_GNSS_v3.h>
#include "board.h"
#include "parameters.h"

typedef struct {
    int32_t lat; // latitude
    int32_t lon; // longitude
    uint8_t siv; // Satelite in view
} GNSS_t;

void setupGNSS(void);
void getGNSS(GNSS_t *gnssVal);

#endif

