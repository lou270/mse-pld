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
// SFE_UBLOX_GNSS gnss;
bool gnssInitialised = false;

bool setupGNSS() {
    #if DEBUG == true
    Serial.println(F("[GNSS] Initializing..."));
    // gnss.enableDebugging(); // Uncomment this line to enable debug messages on Serial
    #endif

    uint8_t gnssTry = 5;

    do {
        #if DEBUG == true
        Serial.println(F("[GNSS] trying 115200 baud"));
        #endif
        Serial2.begin(115200);
        if (gnss.begin(Serial2) == true) {
            gnssInitialised = true;
            break;
        }
        delay(100);
        
        #if DEBUG == true
        Serial.println(F("[GNSS] trying 9600 baud"));
        #endif

        Serial2.begin(9600);
        if (gnss.begin(Serial2) == true) {
            #if DEBUG == true
            Serial.println(F("[GNSS] u-blox MAX10S detected"));
            Serial.println("[GNSS] Connected at 9600 baud, switching to 115200");
            #endif
            
            gnss.setSerialRate(115200);
            delay(100);
        } else {
            //gnss.factoryDefault();
            delay(2000); //Wait a bit before trying again to limit the Serial output
        }
    } while(gnssTry--);

    if (gnssInitialised == true) {
        // gnss.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX messages
        gnss.setUART1Output(COM_TYPE_UBX); //Set the UART port to output UBX only
        gnss.setNavigationFrequency(FREQ_DATA_GNSS);
        gnss.setAutoPVT(true);
        gnss.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

        #if DEBUG == true
        // gnss.setUBXOutputPort(Serial);
        Serial.println(F("[GNSS] u-blox MAX10S done init."));
        #endif
    }

    return gnssInitialised;
}

void getGNSS(GNSS_t* gnssData) {
    if (gnssInitialised == true) {
        #if DEBUG == true
        Serial.print(F("[MAX10S] Checking gps data...\n"));
        #endif
        // if(gnss.getPVT()) {
            gnssData->lat = gnss.getLatitude();
            gnssData->lon = gnss.getLongitude();
            gnssData->alt = gnss.getAltitudeMSL();
            gnssData->siv = gnss.getSIV();
            #if DEBUG == true
            Serial.printf("[MAX10S] Lat: %.7f | Lon: %.7f | Alt: %d | SiV: %d\n", gnssData->lat*1e-7, gnssData->lon*1e-7, gnssData->alt, gnssData->siv);
            #endif
        // }
    }
}

