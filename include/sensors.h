/******************************
| Project       : MSE Avionics
| Board         : SEQ
| Description   : Sensors header
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#ifndef SENSORS_HEADER_FILE
#define SENSORS_HEADER_FILE

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "board.h"
#include "parameters.h"
#include "ADS1x1x.h"

// void setupSensors(void);
void setupSensorAdc(void);
void getSensorADCValue(int16_t *adcValue);

#endif

