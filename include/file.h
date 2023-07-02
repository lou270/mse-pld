/******************************
| Project       : MSE Avionics
| Board         : SEQ
| Description   : File management header
| Licence       : CC BY-NC-SA 
| https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
******************************/
#ifndef FILE_HEADER_FILE
#define FILE_HEADER_FILE

#include <Arduino.h>
#include <pico/time.h>
#include "board.h"
#include "parameters.h"
#include <LittleFS.h>
#include <SingleFileDrive.h>

void setupFileSystem();
void usbPlugedCallback(uint32_t data);
void usbUnplugedCallback(uint32_t data);
void usbDeleteFileCallback(uint32_t data);
bool dataWriterCallback(struct repeating_timer *t);

#endif
