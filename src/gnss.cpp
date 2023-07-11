/******************************
| Project       : MSE Avionics
| Board         : PLD
| Description   : GNSS
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#include "gnss.h"

// GNSS
SFE_UBLOX_GNSS_SERIAL gnss;

void setupGNSS() {
    #if DEBUG == true
    Serial.println(F("[GNSS] Initializing..."));
    // gnss.enableDebugging(); // Uncomment this line to enable debug messages on Serial
    #endif
    
    // if (gnss.begin() == false)
    // {
    //     Serial.println(F("[GNSS] /!\\ u-blox GNSS not detected at default I2C address"));
    // } else {
    //     Serial.println(F("[GNSS] u-blox MAX10S detected"));
    //     delay(100);
    // }

    // gnss.connectedToUART2();
    do {
      Serial.println(F("[GNSS] trying 38400 baud"));
      Serial2.begin(115200);
      if (gnss.begin(Serial2) == true) 
        break;
      delay(100);
      Serial.println(F("[GNSS] trying 9600 baud"));
      Serial2.begin(9600);
      if (gnss.begin(Serial2) == true) {
        Serial.println(F("[GNSS] u-blox MAX10S detected"));
        Serial.println("[GNSS] Connected at 9600 baud, switching to 38400");
        gnss.setSerialRate(115200);
        delay(100);
      } else {
        //gnss.factoryDefault();
        delay(2000); //Wait a bit before trying again to limit the Serial output
      }
    } while(1);

    // gnss.setI2COutput(COM_TYPE_NMEA); //Set the I2C port to output both NMEA and UBX messages
    gnss.setUART1Output(COM_TYPE_UBX); //Set the UART port to output NMEA only
    gnss.setNavigationFrequency(2);
    gnss.setAutoPVT(true);
    // gnss.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

    #if DEBUG == true
    // gnss.setNMEAOutputPort(Serial);
    Serial.println(F("[GNSS] u-blox MAX10S done init."));
    #endif
}

void getGNSS(GNSS_t *gnssData) {
    #if DEBUG == true
    Serial.print(F("[MAX10S] Checking gps data...\n"));
    #endif
    if(gnss.getPVT()) {
        gnssData->lat = gnss.getLatitude();
        gnssData->lon = gnss.getLongitude();
        gnssData->siv = gnss.getSIV();
        #if DEBUG == true
        Serial.print(F("[MAX10S] Lat: "));
        Serial.print(gnssData->lat*1e-7);
        Serial.print(F(" | Lon: "));
        Serial.print(gnssData->lon*1e-7);
        Serial.print(F(" | SiV: "));
        Serial.println(gnssData->siv);
        #endif
    }
}

