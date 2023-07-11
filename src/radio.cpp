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

// uint8_t frameToSend[16] = {0};
volatile bool sendDoneFlag = true;

void setupRadio(void) {
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

    radio.setPacketSentAction(sendDoneCallback);

    #if DEBUG == true
    Serial.println(F("[RADIO] SX1276 init done"));
    #endif
}

void sendDoneCallback(void) {
    sendDoneFlag = true;
}


uint8_t radioSend(TmData_t* tm) {
    #if DEBUG == true
    Serial.print(F("[SX1276] Start transmitting packet ..."));
    #endif

    uint8_t state;
    if(sendDoneFlag) {
        state = radio.startTransmit((uint8_t*)tm, sizeof(TmData_t));
        if (state == 0) {
            sendDoneFlag = false;
        }
    } else {
        return -1;
    }

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

    return state;
}

uint8_t encodeRocketSts(uint8_t id, uint8_t gnssValid, uint8_t status) {
    return (id & 0x3) << 6 | (gnssValid & 0x1) << 3 | (status & 0x7);
}

