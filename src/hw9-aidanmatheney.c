/*
 * Aidan Matheney
 * aidan.matheney@und.edu
 *
 * CSCI 451 HW9
 *
 *
 * Analysis of output when running in nomutex mode compared to mutex mode:
 *
 * When running my HW9 program in nomutex mode, I observed that the word order in the output file was inconsistent from
 * run to run. I noticed that the first 10 words were sometimes printed out of order. This behavior is explained by
 * my program's thread launching procedure. After my program starts, 10 threads are created and immediately begin
 * processing words. Due to random CPU timing and no synchronization, sometimes one thread is able to read and write a
 * word in the time between another thread's reading and writing. After each thread processes its first word, it sleeps
 * for a random duration. These sleeps make future simultaneous processing unlikely, which explains the consistency I
 * observed in the later lines of the output file. The thread number that wrote each word was not consistent between
 * runs. When testing this mode without the sleep, I observed wildly inconsistent ordering, blank lines, and mangled
 * words. I attribute these to the constant concurrent usage of the input and output files, both shared resources,
 * causing operations on different threads to be interleaved.
 *
 * When running my HW9 program in mutex mode, I observed that the word order in the output file was consistent from run
 * to run. The word order matched that of the input file. The thread number that wrote each word did however change each
 * run. Though my program includes a sleep of random duration after each thread processes a word, I observed that even
 * when removing this sleep, the thread numbers continued to be inconsistent from run to run (however, without the
 * sleep, the thread numbers were less randomly distributed and more clustered, since constant processing allows a
 * thread to sometimes immediately reacquire the mutex after releasing it).
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
