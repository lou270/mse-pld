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
* Add sensor data acquisition
*/

// LED status
typedef enum {NO_LED = 0, BLINK_LED = 1, FIXED_LED = 2} LedStatus_t;
enum {RED_LED = 0, GREEN_LED = 1, BLUE_LED = 2};
const uint8_t ledPins[3] = {RGB_LED_R_PIN, RGB_LED_G_PIN, RGB_LED_B_PIN};
volatile LedStatus_t ledStatus[3] = {NO_LED};
struct repeating_timer ledStatusTimer;
volatile bool ledValBlink = 0;

// Utils
uint64_t currTime = 0;
uint64_t prevTimeRadio, prevTimeGnss = 0;

// SEQ-PLD comms
uint8_t seqData[1];

// TM
TmData_t radioData;
// struct repeating_timer radioTimer;

// Sensors and status
typedef enum {PRE_FLIGHT = 0, ASCEND = 1, DESCEND = 2, TOUCHDOWN = 3} RocketStatus_t;
RocketStatus_t rocketSts = PRE_FLIGHT;
bool launchDetected = false;
GNSS_t gnssData;
bool gnssValid = 0;
Imu_t imuData;
Angle_t angleImu = {
    .x = 0.0,
    .y = 0.0,
    .z = 0.0,
};
LPS22HB lps22hb(Wire1); // Pressure sensor
    int32_t pressure; // ambiant pressure
    int16_t temp; // ambiant temperature 
int16_t adcValue[2] = {0};

// Save data
DataFile_t dataFile;

bool ledStatusCallback(struct repeating_timer *t);

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
    Serial1.setTimeout(100); // [ms]
    Serial1.begin(115200);

    // Serial2 (dedicated to GNSS)
    // Initialised in setupGNSS()

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

bool ledStatusCallback(struct repeating_timer *t) {
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
    setupSensorAdc();
    setupIMU();
    lps22hb.begin();
}

void setup() {
    delay(1000);

    setupBoard();
    setupGNSS();
    setupRadio();
    setupSensors();

    // Init LEDs
    ledStatus[GREEN_LED] = FIXED_LED;
    add_repeating_timer_ms(500, ledStatusCallback, NULL, &ledStatusTimer);
}

bool getDelayNonBlocking(uint64_t* cTime, uint64_t* pTime, uint64_t delay) {
    if (((*cTime-*pTime) / (rp2040.f_cpu()/1000.0)) > delay) {
        *pTime = *cTime;
        return true;
    } else {
        return false;
    }
}

void loop() {
    currTime = rp2040.getCycleCount64();

    if (Serial1.available()) {
        Serial1.readBytes(seqData, 1);
        rocketSts = (RocketStatus_t)(seqData[0] & 0x7);
        if (rocketSts == PRE_FLIGHT) {
            ledStatus[GREEN_LED] = FIXED_LED;
        } else {
            ledStatus[GREEN_LED] = BLINK_LED;
        }
    }

    // Get sensor data
    if (getDelayNonBlocking(&currTime, &prevTimeGnss, 1000/FREQ_SENSOR_ACQ)) {
        // TODO get data
        // dataFile.
    }

    // Get data from GNSS
    if (getDelayNonBlocking(&currTime, &prevTimeGnss, 1000/FREQ_DATA_GNSS)) {
        getGNSS(&gnssData);
        if (gnssData.siv > 0) {
            ledStatus[BLUE_LED] = FIXED_LED;
            gnssValid = 1;
        } else {
            ledStatus[BLUE_LED] = BLINK_LED;
            gnssValid = 0;
        }
    }

    // Send TM
    if (getDelayNonBlocking(&currTime, &prevTimeRadio, 1000/FREQ_SEND_TM)) {       
        uint8_t id = 0;
        #if defined(MSE)
        id = 1;
        #elif defined(KRYPTONIT)
        id = 2;
        #endif
        radioData.rocketSts     = encodeRocketSts(id, gnssValid, (uint8_t)rocketSts);
        radioData.gnssLat       = gnssData.lat;
        radioData.gnssLon       = gnssData.lon;
        radioData.gnssAlt       = gnssData.alt;
        radioData.pressure      = lps22hb.readRawPressure();
        radioData.temp          = lps22hb.readRawTemperature();
        radioData.accX          = imuData.ax;
        radioData.accY          = imuData.ay;
        radioData.accZ          = imuData.az;
        radioData.angleX        = angleImu.x;
        radioData.angleY        = angleImu.y;
        radioData.angleZ        = angleImu.z;
        radioData.sensorAdc0    = adcValue[0];
        radioData.sensorAdc1    = adcValue[1];

        radioSend(&radioData);
    }
}

void setup1(void) {
    setupFileSystem();
}

void loop1(void) {
    uint32_t pDataFile;
    if (rp2040.fifo.pop_nb(&pDataFile)) {
        writeDataToBufferFile((DataFile_t*)pDataFile);
    }
}

