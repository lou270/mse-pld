/******************************
| Project       : MSE Avionics
| Board         : PLD
| Description   : Payload main code
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <RadioLib.h>
#include <SparkFun_u-blox_GNSS_v3.h>

#include "board.h"
#include "parameters.h"

/* TODO
* Add communication SEQ<->PLD
* Add sensor data saving
* Add protocol to send data through radio
*/

// GNSS
SFE_UBLOX_GNSS gnss;

// Radio LORA
SX1276 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RESET_PIN, RADIO_DIO0_PIN, SPI1, RADIOLIB_DEFAULT_SPI_SETTINGS);

// SEQ<->PLD UART (from PIO UART)
// SerialPIO SeqPldCom(SEQ_PLD_UART_TX_PIN, SEQ_PLD_UART_RX_PIN, 64);

typedef enum {NO_LED, BLINK_LED, FIXED_LED} ledStatusType;
enum ledName {RED_LED = 0, GREEN_LED = 1, BLUE_LED = 2};
ledStatusType ledStatus[3] = {NO_LED};
uint8_t ledPins[3] = {RGB_LED_R_PIN, RGB_LED_G_PIN, RGB_LED_B_PIN};
struct repeating_timer blinkTimer;
bool launchDetected = false;

typedef struct {
    uint8_t id;
    uint8_t sts;
    int32_t lat;
    int32_t lon;
    uint16_t amb_pressure;
    uint16_t annex0;
    uint16_t annex1;
} TmFrame_t;

TmFrame_t radioFrame;

void initBoard() {
    // USB UART
    Serial.begin();

    // SEQ<->PLD UART (Serial1 = UART0)
    Serial1.setRX(SEQ_PLD_UART_RX_PIN);
    Serial1.setTX(SEQ_PLD_UART_TX_PIN);
    Serial1.setFIFOSize(128);
    Serial1.begin(115200);

    // Init SPI1 (dedicated to radio)
    SPI1.setRX(SPI1_MISO);
    SPI1.setSCK(SPI1_SCLK);
    SPI1.setTX(SPI1_MOSI);
    SPI1.begin();

    // Init I2C0 (dedicated to GNSS)
    Wire.setSDA(I2C0_SDA);
    Wire.setSCL(I2C0_SCL);
    Wire.setClock(400000);
    Wire.begin();

    // Init LED pins
    for(uint8_t iStatus = 0; iStatus < 3; iStatus++) {
        pinMode(ledPins[iStatus], OUTPUT);
        digitalWrite(ledPins[iStatus], 1);
    }
}

void initGNSS() {
    #if DEBUG == true
    Serial.println(F("[GNSS] Initializing..."));
    // gnss.enableDebugging(); // Uncomment this line to enable debug messages on Serial
    #endif
    if (gnss.begin() == false)
    {
        Serial.println(F("[GNSS] /!\\ u-blox GNSS not detected at default I2C address"));
    } else {
        Serial.println(F("[GNSS] u-blox MAX10S detected"));
        delay(100);
    }

    Serial2.setRX(GNSS_UART_RX_PIN);
    Serial2.setTX(GNSS_UART_TX_PIN);

    // gnss.connectedToUART2();
    // do {
    //   Serial.println(F("[GNSS] trying 38400 baud"));
    //   Serial2.begin(38400);
    //   if (gnss.begin(Serial2) == true) 
    //     break;
    //   delay(100);
    //   Serial.println(F("[GNSS] trying 9600 baud"));
    //   Serial2.begin(9600);
    //   if (gnss.begin(Serial1) == true) {
    //     Serial.println(F("[GNSS] u-blox MAX10S detected"));
    //     Serial.println("[GNSS] Connected at 9600 baud, switching to 38400");
    //     gnss.setSerialRate(38400);
    //     delay(100);
    //   } else {
    //     //gnss.factoryDefault();
    //     delay(2000); //Wait a bit before trying again to limit the Serial output
    //   }
    // } while(1);

    gnss.setI2COutput(COM_TYPE_NMEA); //Set the I2C port to output both NMEA and UBX messages
    gnss.setUART1Output(COM_TYPE_NMEA); //Set the UART port to output NMEA only
    gnss.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

    #if DEBUG == true
    // gnss.setNMEAOutputPort(Serial);
    Serial.println(F("[GNSS] u-blox MAX10S done init."));
    #endif
}

void initRadio() {
    // Radio init
    Serial.print(F("[RADIO] Initializing... "));
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
            delay(1000);
        }
    } while(state);


#if DEBUG == true
    Serial.println(F("[RADIO] SX1276 begin init."));
    #endif
}

bool blinkCallback(struct repeating_timer *t) {
    for(uint8_t iStatus = 0; iStatus < 3; iStatus++) {
        if(ledStatus[iStatus] == NO_LED) {
            digitalWrite(ledPins[iStatus], 1);
        } else if (ledStatus[iStatus] == BLINK_LED) {
            digitalWrite(ledPins[iStatus], !digitalRead(ledPins[iStatus]));
        } else {
            digitalWrite(ledPins[iStatus], 0);
        }   
    }
    return true;
}

void setup() {
    initBoard();
    Serial.print(F("Initializing ... "));
    ledStatus[GREEN_LED] = BLINK_LED;
    add_repeating_timer_ms(500, blinkCallback, NULL, &blinkTimer);

    initGNSS();
    initRadio();

    ledStatus[GREEN_LED] = FIXED_LED;
}

int8_t val = 0;

void loop() {

    // while(Serial.)
    // while(!launchDetected) {
    //     // wait to receive launch detection
    //     // TODO launch detect

    //     // temporary
    //     delay(2000);
    //     launchDetected = true;
    //     // ledStatus[1] = FIXED_LED;
    // }

    Serial.print(F("[MAX10S] Checking gps data...\n"));

    int32_t lat = gnss.getLatitude();
    int32_t lon = gnss.getLongitude();
    byte siv = gnss.getSIV();
    Serial.print(F("[MAX10S] Lat : "));
    Serial.print(lat);
    Serial.print(F(" | Lon : "));
    Serial.print(lon);
    Serial.print(F(" | SIV : "));
    Serial.println(siv);

    if (siv > 0) {
        ledStatus[BLUE_LED] = FIXED_LED;
    } else {
        ledStatus[BLUE_LED] = BLINK_LED;
    }

    Serial.print(F("[SX1276] Transmitting packet ... "));

    radioFrame.id = 1;
    radioFrame.sts = 0 + 1 << 2;
    radioFrame.lat = lat;
    radioFrame.lon = lon;
    radioFrame.amb_pressure = 63251;
    radioFrame.annex0 = 45;
    radioFrame.annex1 = 87;

    uint8_t frame[16] = {0};
    frame[0] = radioFrame.id;
    frame[1] = radioFrame.sts;
    frame[2] = radioFrame.lat >> 24 & 0xFF;
    frame[3] = radioFrame.lat >> 16 & 0xFF;
    frame[4] = radioFrame.lat >> 8 & 0xFF;
    frame[5] = radioFrame.lat >> 0 & 0xFF;
    frame[6] = radioFrame.lon >> 24 & 0xFF;
    frame[7] = radioFrame.lon >> 16 & 0xFF;
    frame[8] = radioFrame.lon >> 8 & 0xFF;
    frame[9] = radioFrame.lon >> 0 & 0xFF;
    frame[10] = radioFrame.amb_pressure >> 8 & 0xFF;
    frame[11] = radioFrame.amb_pressure >> 0 & 0xFF;
    frame[12] = radioFrame.annex0 >> 8 & 0xFF;
    frame[13] = radioFrame.annex0 >> 0 & 0xFF;
    frame[14] = radioFrame.annex1 >> 8 & 0xFF;
    frame[15] = radioFrame.annex1 >> 0 & 0xFF;

    int state = radio.transmit(frame, 16);

    if (state == RADIOLIB_ERR_NONE) {
        // the packet was successfully transmitted
        Serial.println(F(" success!"));

        // print measured data rate 
        Serial.print(F("[SX1276] Datarate:\t"));
        Serial.print(radio.getDataRate());
        Serial.println(F(" bps"));

    } else if (state == RADIOLIB_ERR_PACKET_TOO_LONG) {
        // the supplied packet was longer than 256 bytes
        Serial.println(F("too long!"));

    } else if (state == RADIOLIB_ERR_TX_TIMEOUT) {
        // timeout occurred while transmitting packet
        Serial.println(F("timeout!"));

    } else {
        // some other error occurred
        Serial.print(F("failed, code "));
        Serial.println(state);

    }

    // delay(2000);
}

