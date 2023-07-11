/******************************
| Project       : MSE Avionics
| Board         : PLD
| Description   : Payload main code
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#include <Arduino.h>
#include <pico/time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h>
#include <RadioLib.h>
#include <SparkFun_u-blox_GNSS_v3.h>
#include "board.h"
#include "parameters.h"
#include "sensors.h"
#include "imu.h"
#include "radio.h"
#include "gnss.h"
#include "file.h"

/* TODO
* Add communication SEQ<->PLD
* Add sensor data saving
*/

// Pressure sensor
LPS22HB lps22hb(Wire1);

// SEQ<->PLD UART (from PIO UART)
// SerialPIO SeqPldCom(SEQ_PLD_UART_TX_PIN, SEQ_PLD_UART_RX_PIN, 64);

typedef enum {NO_LED = 0, BLINK_LED = 1, FIXED_LED = 2} LedStatus_t;
enum {RED_LED = 0, GREEN_LED = 1, BLUE_LED = 2};
const uint8_t ledPins[3] = {RGB_LED_R_PIN, RGB_LED_G_PIN, RGB_LED_B_PIN};
volatile LedStatus_t ledStatus[3] = {NO_LED};
struct repeating_timer userStatusTimer;

struct repeating_timer radioTimer;
bool launchDetected = false;

uint64_t currTime, prevTime = 0;

TmData_t radioData;
GNSS_t gnssData;
bool gnssValid = 0;
int16_t adcValue[2] = {0};

Angle_t angleImu = {
    .x = 0.0,
    .y = 0.0,
    .z = 0.0,
};

volatile bool ledValBlink = 0;
bool userStatusCallback(struct repeating_timer *t);

void setupBoard() {
    #if DEBUG == true
    // USB UART
    Serial.begin(115200);
    Serial.print(F("Initializing ... "));
    #endif

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

    // Init I2C0 (dedicated to GNSS and sensor board)
    Wire.setSDA(I2C0_SDA);
    Wire.setSCL(I2C0_SCL);
    Wire.setClock(100000);
    Wire.begin();

    // Init I2C1 (dedicated to IMU)
    Wire1.setSDA(I2C1_SDA);
    Wire1.setSCL(I2C1_SCL);
    Wire1.setClock(400000);
    Wire1.begin();

    // Init Serial2 for GNSS
    Serial2.setRX(GNSS_UART_RX_PIN);
    Serial2.setTX(GNSS_UART_TX_PIN);
    Serial2.setFIFOSize(128);
    Serial2.begin(115200);

    // Init LED pins
    for(uint8_t iLed = 0; iLed < 3; iLed++) {
        pinMode(ledPins[iLed], OUTPUT);
        digitalWrite(ledPins[iLed], 1);
    }

    pinMode(PICO_BUTTON_PIN, INPUT_PULLUP);
}

bool userStatusCallback(struct repeating_timer *t) {
    ledValBlink = !ledValBlink;
    for(uint8_t iLed = 0; iLed < 3; iLed++) {
        // Serial.print(F("LED "));
        // Serial.print(iLed);
        // Serial.print(" : ");
        // Serial.print(ledStatus[iLed]);
        // Serial.println("");
        if(ledStatus[iLed] == NO_LED) {
            // Serial.println("NO LED");
            digitalWrite(ledPins[iLed], 1);
        } else if (ledStatus[iLed] == BLINK_LED) {
            // Serial.println("BLINK LED");
            digitalWrite(ledPins[iLed], ledValBlink);
        } else if (ledStatus[iLed] == FIXED_LED) {
            // Serial.println("FIXED LED");
            digitalWrite(ledPins[iLed], 0);
        } else {
            // Serial.println("BIG BUG");
        }
    }
    return true;
}

void setupSensors(void) {
    //initSensorADC();
    //setupIMU();
    lps22hb.begin();
}

void setup() {
    delay(1000);
    rp2040.idleOtherCore();
    setupBoard();
    setupGNSS();
    setupRadio();
    setupSensors();
    setupFileSystem();
    ledStatus[GREEN_LED] = FIXED_LED;
    add_repeating_timer_ms(500, userStatusCallback, NULL, &userStatusTimer);
    // add_repeating_timer_ms(500, sendDataCallback, NULL, &radioTimer);
}

void loop() {
    currTime = rp2040.getCycleCount64();

    // while(Serial.)
    // while(!launchDetected) {
    //     // wait to receive launch detection
    //     // TODO launch detect

    //     // temporary
    //     delay(2000);
    //     launchDetected = true;
    //     // ledStatus[1] = FIXED_LED;
    // }


    Serial.print("PIN : ");
    Serial.println(digitalRead(PICO_BUTTON_PIN));
    delay(500);

    if (((currTime-prevTime) / (rp2040.f_cpu()/1000.0)) > 500) {
        prevTime = currTime;
        
        DataFile_t dataFile = {
            .rocketSts   = 0,
            .gnssLat = 0,
            .gnssLon = 1,
            .gnssAlt = 1,
            .pressure    = 0,
            .temperature = 0,
            .accX    = 0,
            .accY    = 0,
            .accZ    = 0,
            .gyrX    = 0,
            .gyrY    = 0,
            .gyrZ    = 0,
            .sensorAdc0  = 0,
            .sensorAdc1  = 1,
        };
        writeDataToBufferFile(&dataFile);
        // writeBufferToFile(&dataFile, sizeof(DataFile_t));

        getGNSS(&gnssData);
        if (gnssData.siv > 0) {
            ledStatus[BLUE_LED] = FIXED_LED;
            gnssValid = 1;
        } else {
            ledStatus[BLUE_LED] = BLINK_LED;
            gnssValid = 0;
        }

        // for(uint8_t iLed = 0; iLed < 3; iLed++) {
        //     Serial.print("LED ");
        //     Serial.print(iLed);
        //     Serial.print(" : ");
        //     Serial.println(ledStatus[iLed]);
        // }

        // Serial.println(lps22hb.readPressure());
        // Serial.println(lps22hb.readTemperature());

        // getSensorADCValue(adcValue);
        Serial.print("[ADC] ADC0 : ");
        Serial.println(adcValue[0]*3);
        Serial.print("[ADC] ADC1 : ");
        Serial.println(adcValue[1]*3);
        Serial.print("[PRESSURE] AMB : ");
        Serial.println(lps22hb.readPressure());

        #if defined(MSE)
        radioData.id = 1;
        #elif defined(KRYPTONIT)
        radioData.id = 2;
        #endif
        radioData.rocketSts = 0;
        radioData.gnssValid = gnssValid;
        radioData.lat = gnssData.lat;
        radioData.lon = gnssData.lon;
        // radioData.pressure = lps22hb.readRawPressure();
        // radioData.temp = lps22hb.readRawTemperature();
        radioData.annex0 = adcValue[0];
        radioData.annex1 = adcValue[1];
        radioData.angleX = angleImu.x;
        radioData.angleY = angleImu.y;
        radioData.angleZ = angleImu.z;

        radioSend(&radioData);
    }
    // delay(2000);
}

// void setup1(void) {
//     // setupFileSystem();
// }

// void loop1(void) {

// }

