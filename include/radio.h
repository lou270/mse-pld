/******************************
| Project       : MSE Avionics
| Board         : SEQ
| Description   : Radio header
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#ifndef RADIO_HEADER_FILE
#define RADIO_HEADER_FILE

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>
#include "board.h"
#include "parameters.h"

typedef struct {
    int32_t pressure; // ambiant pressure
    int32_t gnssLat; // GNSS latitute
    int32_t gnssLon; // GNSS longitude
    int16_t gnssAlt; // GNSS altitude
    int16_t temp; // ambiant temperature 
    int16_t accX; // acceleration X
    int16_t accY; // acceleration Y
    int16_t accZ; // acceleration Z
    int16_t sensorAdc0; // Sensor board ADC0
    int16_t sensorAdc1; // Sensor board ADC1
    uint8_t rocketSts; // status of rocket
} TmData_t;

bool setupRadio(void); 
void sendDoneCallback(void);
int16_t radioSend(TmData_t* tm);
uint8_t encodeRocketSts(uint8_t id, uint8_t gnssValid, uint8_t status);

#endif

