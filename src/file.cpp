/******************************
| Project       : MSE Avionics
| Board         : SEQ
| Description   : File management
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#include "file.h"

volatile bool usbPluged = false;

void usbPlugedCallback(uint32_t data) {
    // Tell my app not to write to flash, we're connected
    // Serial.println("[FileSystem] USB mounted");
}

void usbUnplugedCallback(uint32_t data) {
    // I can start writing to flash again
    // Serial.println("[FileSystem] USB unmounted");
}

void usbDeleteFileCallback(uint32_t data) {
    // Maybe LittleFS.remove("myfile.txt")?  or do nothing
    // Serial.print("[FileSystem] Delete file: ");
    // Serial.println(data);
}

void setupFileSystem() {
    LittleFSConfig cfg;
    cfg.setAutoFormat(false);
    LittleFS.setConfig(cfg);
    Serial.println(LittleFS.begin());
    singleFileDrive.onPlug(usbPlugedCallback);
    singleFileDrive.onUnplug(usbUnplugedCallback);
    singleFileDrive.onDelete(usbDeleteFileCallback);
    Serial.println(singleFileDrive.begin("littlefsfile.csv", "DataRecorder.csv"));
    File f = LittleFS.open("littlefsfile.csv", "w");
    f.printf("1DSFSFG");
    f.close();
    interrupts();
}

bool dataWriterCallback(struct repeating_timer *t) {
    //   boolean wut = digitalRead(PICO_LED_PIN);
    //   digitalWrite(PICO_LED_PIN,!wut);
    return true;
}


