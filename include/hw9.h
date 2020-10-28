#pragma once

enum HW9Mode {
    HW9Mode_Mutex,
    HW9Mode_NoMutex
};
enum HW9Mode HW9Mode_parse(char const *name);

void hw9(char const *inFilePath, char const *outFilePath, enum HW9Mode mode, unsigned int threadCount);
