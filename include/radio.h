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
    uint8_t id; // Unique ID of project
    uint8_t rocketSts; // status of rocket
    bool gnssValid; // status of rocket
    int32_t lat; // GNSS latitute
    int32_t lon; // GNSS longitude
    int32_t pressure; // ambiant pressure
    int16_t temp; // ambiant temperature 
    int16_t annex0; // ADC0
    int16_t annex1; // ADC1
    int16_t angleX; // euler angle X
    int16_t angleY; // euler angle Y
    int16_t angleZ; // euler angle Z
} TmData_t;

void setupRadio(void); 
uint8_t radioSend(TmData_t *tm);
void encodeFrame(TmData_t *tm);
void sendDoneCallback(void);

#endif

