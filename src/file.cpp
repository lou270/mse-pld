/******************************
| Project       : MSE Avionics
| Board         : SEQ
| Description   : File management
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#include "file.h"

FSInfo64 fsinfo;

uint8_t bufFile[sizeof(DataFile_t)*(uint16_t)(FLASH_SECTOR_SIZE/sizeof(DataFile_t))] = {0};
uint16_t currOffBufFile = 0;

void setupFileSystem() {
    #if DEBUG == true
    Serial.print("[FS] Initialisation ... ");
    #endif

    LittleFSConfig cfg;
    cfg.setAutoFormat(false);
    LittleFS.setConfig(cfg);
    LittleFS.begin();

    #if DEBUG == true
    Serial.println("done");
    #endif
}

void writeDataToBufferFile(DataFile_t* df) {
    memcpy(bufFile+(currOffBufFile*sizeof(DataFile_t)), df, sizeof(DataFile_t));
    if (currOffBufFile >= (uint16_t)(FLASH_SECTOR_SIZE/sizeof(DataFile_t))-1) {
        currOffBufFile = 0;
        writeBufferToFile(bufFile, sizeof(bufFile));
    } else {
        currOffBufFile++;
    }
}

void writeBufferToFile(void *buffer, uint32_t bufSize) {
    uint64_t t0 = rp2040.getCycleCount64();
    noInterrupts();
    File f = LittleFS.open("testData2.bin", "a");
    f.write((uint8_t*) buffer, bufSize);
    f.close();
    interrupts();
    uint64_t t1 = rp2040.getCycleCount64();
    Serial.printf("\n[FILE] Write in : %d \n\n", (t1-t0));
}
