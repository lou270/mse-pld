/******************************
| Project       : MSE Avionics
| Board         : PLD
| Description   : Payload main code
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#include <Arduino.h>
#include <pico/time.h>
#include "hardware/flash.h"
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
uint8_t seqData;

// TM
TmData_t radioData;
// struct repeating_timer radioTimer;

// Sensors and status
typedef enum {PRE_FLIGHT = 0, ASCEND, DEPLOY_ALGO, DEPLOY_TIMER, DESCEND, TOUCHDOWN} RocketState_t;
RocketState_t rocketSts = PRE_FLIGHT;
volatile bool launchDetected = false;
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

bool setupBoard() {
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
    Wire.setClock(400000);
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

    // Setup LED Pico
    pinMode(PICO_LED_PIN, OUTPUT);
    digitalWrite(PICO_LED_PIN, LOW);

    // Setup button Pico
    pinMode(PICO_BUTTON_PIN, INPUT_PULLUP);

    return true;
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

bool setupSensors(void) {
    bool ret = true;
    ret &= setupSensorAdc();
    ret &= setupIMU();
    ret &= lps22hb.begin();

    return ret;
}

void setup() {
    delay(2000);

    bool ret = true;
    ret &= setupBoard();
    ret &= setupGNSS();
    ret &= setupRadio();
    ret &= setupSensors();

    // Init LEDs
    if (ret == false) {
        ledStatus[RED_LED] = FIXED_LED;
    } else {
        ledStatus[GREEN_LED] = FIXED_LED;
    }
    add_repeating_timer_ms(500, ledStatusCallback, NULL, &ledStatusTimer);
}

bool getDelayNonBlocking(uint64_t* cTime, uint64_t* pTime, float delay) {
    if (((*cTime-*pTime) * 1.0 / (rp2040.f_cpu()/1000.0)) > delay) {
        *pTime = *cTime;
        return true;
    } else {
        return false;
    }
}

void loop() {
    currTime = rp2040.getCycleCount64();

    // Get status from sequencer
    if (Serial1.available()) {
        Serial1.readBytes(&seqData, sizeof(seqData));
        rocketSts = (RocketState_t)(seqData);
        if (rocketSts == PRE_FLIGHT) {
            ledStatus[GREEN_LED] = FIXED_LED;
        } else {
            launchDetected = true;
            ledStatus[GREEN_LED] = BLINK_LED;
        }
    }

    // Get data from GNSS
    if (getDelayNonBlocking(&currTime, &prevTimeGnss, 1000.0/FREQ_DATA_GNSS)) {
        getGNSS(&gnssData);
        dataFile.gnssLat = gnssData.lat;
        dataFile.gnssLon = gnssData.lon;
        dataFile.gnssAlt = gnssData.alt;
        if (gnssData.siv > 0) {
            ledStatus[BLUE_LED] = FIXED_LED;
            gnssValid = 1;
        } else {
            ledStatus[BLUE_LED] = BLINK_LED;
            gnssValid = 0;
        }
    }

    // Get sensor data
    if (getDelayNonBlocking(&currTime, &prevTimeGnss, 1000.0/FREQ_SENSOR_ACQ)) {
        dataFile.rocketSts = (currTime << 8) & 0xFFFFFFFFFFFFFF | encodeRocketSts(PROJECT_ID, gnssValid, (uint8_t)rocketSts) & 0xFF;
        dataFile.pressure = lps22hb.readRawPressure();
        dataFile.temperature = lps22hb.readRawTemperature();
        getImuData(&imuData, false);
        computeAngle(&imuData, &angleImu);
        dataFile.accX = imuData.raw_ax;
        dataFile.accY = imuData.raw_ay;
        dataFile.accZ = imuData.raw_az;
        dataFile.gyrX = imuData.raw_gx;
        dataFile.gyrY = imuData.raw_gy;
        dataFile.gyrZ = imuData.raw_gz;
        getSensorADCValue(adcValue);
        dataFile.sensorAdc0 = adcValue[0];
        dataFile.sensorAdc1 = adcValue[1];

        if (rocketSts != PRE_FLIGHT) {
            writeDataToBufferFile(&dataFile);
        } else {
            // writeDataToPreFlightBufferFile(&dataFile); 
        }
    }

    // Send TM
    if (getDelayNonBlocking(&currTime, &prevTimeRadio, 1000.0/FREQ_SEND_TM)) {       
        radioData.rocketSts     = encodeRocketSts(PROJECT_ID, gnssValid, (uint8_t)rocketSts) & 0xFF;
        radioData.gnssLat       = gnssData.lat;
        radioData.gnssLon       = gnssData.lon;
        radioData.gnssAlt       = (int16_t)gnssData.alt*1000;
        radioData.pressure      = dataFile.pressure;
        radioData.temp          = dataFile.temperature;
        radioData.accX          = imuData.raw_ax;
        radioData.accY          = imuData.raw_ay;
        radioData.accZ          = imuData.raw_az;
        radioData.sensorAdc0    = adcValue[0];
        radioData.sensorAdc1    = adcValue[1];

        #if DEBUG == true
        Serial.printf("%8X %8X %8X %4X %4X %4X %4X %4X %4X %4X %2X\n", radioData.pressure, radioData.gnssLat, radioData.gnssLon, radioData.gnssAlt, radioData.temp, radioData.accX, radioData.accY, radioData.accZ, radioData.sensorAdc0, radioData.sensorAdc1, radioData.rocketSts);
        #endif

        radioSend(&radioData);
    }

    #if DEBUG == true
    delay(500);
    #endif
}

void setup1(void) {
    setupFileSystem();
}

void loop1(void) {
    uint32_t pBuffer;
    if (rp2040.fifo.pop_nb(&pBuffer)) {
        writeBufferToFile((uint8_t*)pBuffer, sizeof(DataFile_t)*(uint16_t)(FLASH_SECTOR_SIZE/sizeof(DataFile_t)));
    }
}
