/******************************
| Project       : MSE Avionics
| Board         : PLD
| Description   : Sensors management
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#include <sensors.h>

// ADC sensor board
ADS1013 sensorAdc0;
ADS1013 sensorAdc1;

void initSensorADC(void) {
    sensorAdc0.attach(Wire);
    sensorAdc0.setAddress(ADC_0_ADDRESS);
    sensorAdc0.mode(ADS1x1x::ConfigMode::CONTINUOUS);
    sensorAdc0.datarate(ADS1x1x::ConfigDR::DR_12B_0920_SPS);
    sensorAdc0.compMode(ADS1x1x::ConfigCompMode::TRADITIONAL);

    #if defined(KRYPTONIT)
    sensorAdc1.attach(Wire);
    sensorAdc1.setAddress(ADC_1_ADDRESS);
    sensorAdc1.mode(ADS1x1x::ConfigMode::CONTINUOUS);
    sensorAdc1.datarate(ADS1x1x::ConfigDR::DR_12B_0920_SPS);
    sensorAdc1.compMode(ADS1x1x::ConfigCompMode::TRADITIONAL);
    #endif
}

void getSensorADCValue(uint16_t *adcValue) {
    adcValue[0] = sensorAdc0.read();
    #if defined(KRYPTONIT)
    adcValue[1] = sensorAdc1.read();
    #endif
}


