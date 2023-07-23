/******************************
| Project       : MSE Avionics
| Board         : PLD
| Description   : File management
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#include "file.h"

FSInfo64 fsinfo;

volatile uint8_t preBufFile[sizeof(DataFile_t)*(uint16_t)(FLASH_SECTOR_SIZE/sizeof(DataFile_t))] = {0};
volatile uint8_t bufFile0[sizeof(DataFile_t)*(uint16_t)(FLASH_SECTOR_SIZE/sizeof(DataFile_t))] = {0};
volatile uint8_t bufFile1[sizeof(DataFile_t)*(uint16_t)(FLASH_SECTOR_SIZE/sizeof(DataFile_t))] = {0};
volatile uint8_t currBuf = 0;
volatile uint16_t currOffBufFile, currOffPreBufFile = 0;

char filenameToWrite[20] = "";

void setupFileSystem() {
    #if DEBUG == true
    Serial.print("[FS] Initialisation ... ");
    #endif

    LittleFSConfig cfg;
    cfg.setAutoFormat(false);
    LittleFS.setConfig(cfg);

    #if DEBUG == true
    Serial.printf("[FS] Format: %d\n", digitalRead(PICO_BUTTON_PIN));
    #endif

    if(digitalRead(PICO_BUTTON_PIN) == 0) {
        digitalWrite(PICO_LED_PIN, 1);
        // LittleFS.format();
    }

    LittleFS.begin();

    // LittleFS.format();

    // Get incremental filename
    int16_t maxCounter = -1;
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
        String fileName = dir.fileName();

        // Check file starting with file_
        if (fileName.startsWith("file_")) {
            // Get counter at the end
            uint16_t counter = fileName.substring(5, fileName.length()-4).toInt();

            if (counter > maxCounter) {
                maxCounter = counter;
            }
        }
    }
    sprintf(filenameToWrite, "file_%d.bin", maxCounter+1);

    #if DEBUG == true
    Serial.println("done");
    #endif
}

void writeDataToPreFlightBufferFile(DataFile_t* df) {
    memcpy((uint8_t*)preBufFile+(currOffPreBufFile*sizeof(DataFile_t)), df, sizeof(DataFile_t));
    if (currOffPreBufFile >= (uint16_t)(sizeof(preBufFile)/sizeof(DataFile_t))-1) {
        currOffPreBufFile = 0;
    } else {
        currOffPreBufFile++;
    }
}

void flushPreBuffer() {
    uint16_t lenToCpy = sizeof(preBufFile)-(currOffPreBufFile*sizeof(DataFile_t));
    memcpy((uint8_t*)bufFile1, (uint8_t*)preBufFile+(currOffPreBufFile*sizeof(DataFile_t)), lenToCpy);
    memcpy((uint8_t*)bufFile1+lenToCpy, (uint8_t*)preBufFile, (currOffPreBufFile-1)*sizeof(DataFile_t));
    rp2040.fifo.push_nb((uint32_t)bufFile1);
    // uint8_t profBuf = 3-1;
    // do {
    //     if (currOffPreBufFile+(uint16_t)(sizeof(FLASH_SECTOR_SIZE)/sizeof(DataFile_t)) < (preBufFile+sizeof(preBufFile))) {
    //         rp2040.fifo.push_nb((uint32_t)(preBufFile+buf));
    //     }
    // } while(profBuf--)
    // rp2040.fifo.push_nb((uint32_t)(preBufFile+buf));
    // rp2040.fifo.push_nb((uint32_t)(preBufFile+buf));
}

void writeDataToBufferFile(DataFile_t* df) {
    uint8_t* buf;
    if (currBuf == 0) {
        buf = (uint8_t*) bufFile0;
    } else {
        buf = (uint8_t*) bufFile1;
    }
    memcpy(buf+(currOffBufFile*sizeof(DataFile_t)), df, sizeof(DataFile_t));
    if (currOffBufFile >= (uint16_t)(FLASH_SECTOR_SIZE/sizeof(DataFile_t))-1) {
        currOffBufFile = 0;
        currBuf = !currBuf & 0x1;
        rp2040.fifo.push_nb((uint32_t)buf);
    } else {
        currOffBufFile++;
    }
}

void writeBufferToFile(void *buffer, uint32_t bufSize) {
    #if DEBUG == true
    uint64_t t0 = rp2040.getCycleCount64();
    #endif
    noInterrupts();
    File f = LittleFS.open(filenameToWrite, "a");
    f.write((uint8_t*) buffer, bufSize);
    f.close();
    interrupts();
    #if DEBUG == true
    uint64_t t1 = rp2040.getCycleCount64();
    Serial.printf("\n[FILE] Write in : %d \n\n", (t1-t0));
    #endif
}
