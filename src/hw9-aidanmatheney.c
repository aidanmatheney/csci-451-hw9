/*
 * Aidan Matheney
 * aidan.matheney@und.edu
 *
 * CSCI 451 HW9
 */

#include "../include/hw9.h"

#include <stdlib.h>
#include <stdio.h>

int main(int const argc, char ** const argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s mutex|nomutex\n", argv[0]);
        return EXIT_FAILURE;
    }
    enum HW9Mode const hw9Mode = HW9Mode_parse(argv[1]);

    static char const * const inFilePath = "hw9.data";
    char const * const outFilePath = hw9Mode == HW9Mode_Mutex ? "hw9.mutex" : "hw9.nomutex";

    hw9(inFilePath, outFilePath, hw9Mode, 10);
    return EXIT_SUCCESS;
}
