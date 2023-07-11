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
    uint8_t rocketSts; // status of rocket
    int32_t gnssLat; // GNSS latitute
    int32_t gnssLon; // GNSS longitude
    int32_t gnssAlt; // GNSS altitude
    int32_t pressure; // ambiant pressure
    int16_t temp; // ambiant temperature 
    int16_t accX; // acceleration X
    int16_t accY; // acceleration Y
    int16_t accZ; // acceleration Z
    int16_t angleX; // euler angle X
    int16_t angleY; // euler angle Y
    int16_t angleZ; // euler angle Z
    int16_t sensorAdc0; // Sensor board ADC0
    int16_t sensorAdc1; // Sensor board ADC1
} TmData_t;

void setupRadio(void); 
void sendDoneCallback(void);
uint8_t radioSend(TmData_t* tm);
uint8_t encodeRocketSts(uint8_t id, uint8_t gnssValid, uint8_t status);

#endif

