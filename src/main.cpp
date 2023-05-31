/*************
* Project : MSE Avionics
* Board : PAYLOAD
* Description : Board test
**************/

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <RadioLib.h>
#include <SparkFun_u-blox_GNSS_v3.h>

#include "board.h"

// arduino::MbedI2C I2C0(I2C0_SDA, I2C0_SCL);
// arduino::MbedSPI SPI1(SPI1_MISO, SPI1_MOSI, SPI1_SCLK);
SFE_UBLOX_GNSS gnss;
SX1276 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RESET_PIN, RADIO_DIO0_PIN, SPI1, RADIOLIB_DEFAULT_SPI_SETTINGS);

void setup() {
    // SEQ PLD UART
    Serial.begin(115200);

    Serial.print(F("Initializing ... "));

    // SPI Init
    SPI1.setRX(SPI1_MISO);
    SPI1.setSCK(SPI1_SCLK);
    SPI1.setTX(SPI1_MOSI);
    SPI1.begin();

    // Radio init
    Serial.print(F("[RADIO] Initializing ... "));
    int state = radio.begin(868.0);
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true);
    }

    Serial.println(F("[GNSS] Initializing..."));

    // Init I2C
    Wire.setSDA(I2C0_SDA);
    Wire.setSCL(I2C0_SCL);
    Wire.begin();

    // gnss.enableDebugging(); // Uncomment this line to enable debug messages on Serial
    if (gnss.begin() == false)
    {
        Serial.println(F("[GNSS] /!\\ u-blox GNSS not detected at default I2C address"));
    } else {
        Serial.println(F("[GNSS] u-blox MAX10S detected"));
        delay(100);
    }

    // do {
    //   Serial.println(F("[GNSS] trying 38400 baud"));
    //   Serial1.begin(38400);
    //   if (gnss.begin(Serial1) == true) break;

    //   delay(100);
    //   Serial.println(F("[GNSS] trying 9600 baud"));
    //   Serial1.begin(9600);
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

    gnss.setI2COutput(COM_TYPE_UBX | COM_TYPE_NMEA); //Set the I2C port to output both NMEA and UBX messages
    gnss.setUART1Output(COM_TYPE_NMEA); //Set the UART port to output UBX only
    // gnss.saveConfiguration();
    gnss.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

    gnss.setNMEAOutputPort(Serial);

    Serial.println(F("u-blox MAX10S configured"));

    delay(1000);
}

int8_t val = 0;

void loop() {    
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

    Serial.print(F("[SX1276] Transmitting packet ... "));
    char txt[10] = "";
    sprintf(txt, "%4d;%4d", lat, lon);
    int state = radio.transmit(txt);

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
    
    delay(5000);
}





