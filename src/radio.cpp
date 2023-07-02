/******************************
| Project       : MSE Avionics
| Board         : PLD
| Description   : Payload main code
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#include "radio.h"

// Radio LORA
SX1276 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RESET_PIN, RADIO_DIO0_PIN, SPI1, RADIOLIB_DEFAULT_SPI_SETTINGS);

uint8_t frameToSend[16] = {0};
volatile bool sendDoneFlag = true;

void setupRadio() {
    // Radio init
    #if DEBUG == true
    Serial.print(F("[RADIO] Initializing... "));
    #endif
    int state = 0;
    do {
        state = radio.begin(LORA_FREQ);
        state += radio.setBandwidth(LORA_BW);
        state += radio.setOutputPower(LORA_PW);
        state += radio.setCurrentLimit(LORA_OCP);
        state += radio.setSpreadingFactor(LORA_SF);
        state += radio.setCRC(true, false);
        if (state != RADIOLIB_ERR_NONE) {
            #if DEBUG == true
            Serial.print(F("[RADIO] Failed init, code:"));
            Serial.println(state);
            #endif
            delay(100);
        }
    } while(state);

    // radio.setPacketSentAction(sendDoneCallback);

    #if DEBUG == true
    Serial.println(F("[RADIO] SX1276 init done"));
    #endif
}

void sendDoneCallback() {
    sendDoneFlag = true;
}


uint8_t radioSend(TmData_t *tm) {
    #if DEBUG == true
    Serial.print(F("[SX1276] Transmitting packet ... "));
    #endif

    int state;
    // if(sendDoneFlag) {
        encodeFrame(tm);
        state = radio.transmit(frameToSend, 26);
    // } else {
        // return -1;
    // }

    #if DEBUG == true
    if (state == RADIOLIB_ERR_NONE) {
        // the packet was successfully transmitted
        Serial.print(F(" success!"));
        // print measured data rate 
        Serial.print(F(" (Datarate: "));
        Serial.print(radio.getDataRate());
        Serial.print(F(" bps)"));
    } else if (state == RADIOLIB_ERR_PACKET_TOO_LONG) {
        // the supplied packet was longer than 256 bytes
        Serial.print(F("too long!"));
    } else if (state == RADIOLIB_ERR_TX_TIMEOUT) {
        // timeout occurred while transmitting packet
        Serial.print(F("timeout!"));
    } else {
        // some other error occurred
        Serial.print(F("failed, code "));
        Serial.print(state);
    }
    Serial.println(F(""));
    #endif

    return 0;
}

void encodeFrame(TmData_t *tm) {
    frameToSend[0]  = tm->id & 0x3;
    frameToSend[1]  = (tm->gnssValid & 0x1) << 3 + tm->rocketSts;
    frameToSend[2]  = tm->lat >> 24 & 0xFF;
    frameToSend[3]  = tm->lat >> 16 & 0xFF;
    frameToSend[4]  = tm->lat >> 8 & 0xFF;
    frameToSend[5]  = tm->lat >> 0 & 0xFF;
    frameToSend[6]  = tm->lon >> 24 & 0xFF;
    frameToSend[7]  = tm->lon >> 16 & 0xFF;
    frameToSend[8]  = tm->lon >> 8 & 0xFF;
    frameToSend[9]  = tm->lon >> 0 & 0xFF;
    frameToSend[10] = tm->pressure >> 24 & 0xFF;
    frameToSend[11] = tm->pressure >> 16 & 0xFF;
    frameToSend[12] = tm->pressure >> 8 & 0xFF;
    frameToSend[13] = tm->pressure >> 0 & 0xFF;
    frameToSend[14] = tm->temp >> 8 & 0xFF;
    frameToSend[15] = tm->temp >> 0 & 0xFF;
    frameToSend[16] = tm->annex0 >> 8 & 0xFF;
    frameToSend[17] = tm->annex0 >> 0 & 0xFF;
    frameToSend[18] = tm->annex1 >> 8 & 0xFF;
    frameToSend[19] = tm->annex1 >> 0 & 0xFF;
    frameToSend[20] = tm->angleX >> 8 & 0xFF;
    frameToSend[21] = tm->angleX >> 0 & 0xFF;
    frameToSend[22] = tm->angleY >> 8 & 0xFF;
    frameToSend[23] = tm->angleY >> 0 & 0xFF;
    frameToSend[24] = tm->angleZ >> 8 & 0xFF;
    frameToSend[25] = tm->angleZ >> 0 & 0xFF;
}




