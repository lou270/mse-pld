/******************************
| Project       : MSE Avionics
| Board         : PLD
| Description   : Sensors management
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#include <sensors.h>

// ADC sensor board
ADS1014 sensorAdc0;
ADS1014 sensorAdc1;

void setupSensorAdc(void) {
    sensorAdc0.attach(Wire);
    sensorAdc0.setAddress(ADC_0_ADDRESS);
    sensorAdc0.gain(ADS1x1x::ConfigPGA::FSR_6_144V);
    sensorAdc0.mode(ADS1x1x::ConfigMode::CONTINUOUS);
    sensorAdc0.datarate(ADS1x1x::ConfigDR::DR_12B_0920_SPS);
    sensorAdc0.compMode(ADS1x1x::ConfigCompMode::TRADITIONAL);
    sensorAdc0.compQue(ADS1x1x::ConfigCompQue::DISABLE);

    #if defined(KRYPTONIT)
    sensorAdc1.attach(Wire);
    sensorAdc1.setAddress(ADC_1_ADDRESS);
    sensorAdc1.gain(ADS1x1x::ConfigPGA::FSR_6_144V);
    sensorAdc1.mode(ADS1x1x::ConfigMode::CONTINUOUS);
    sensorAdc1.datarate(ADS1x1x::ConfigDR::DR_12B_0920_SPS);
    sensorAdc1.compMode(ADS1x1x::ConfigCompMode::TRADITIONAL);
    sensorAdc0.compQue(ADS1x1x::ConfigCompQue::DISABLE);
    #endif
}

void getSensorADCValue(int16_t *adcValue) {
    adcValue[0] = sensorAdc0.read();
    #if defined(KRYPTONIT)
    adcValue[1] = sensorAdc1.read();
    #endif

    #if DEBUG == true
    Serial.printf("[ADC] ADC0: %d / ADC1: %d\n", adcValue[0], adcValue[1]);
    #endif
}
