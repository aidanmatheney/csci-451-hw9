#include "../include/hw9.h"

#include "../include/util/memory.h"
#include "../include/util/thread.h"
#include "../include/util/file.h"
#include "../include/util/random.h"
#include "../include/util/guard.h"
#include "../include/util/error.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>

struct ProcessWordsWithMutexThreadStartArg {
    unsigned int threadNumber;
    FILE *inFile;
    FILE *outFile;
    pthread_mutex_t *fileMutexPtr;
};
static void *processWordsWithMutexThreadStart(void *argAsVoidPtr);

struct ProcessWordsWithoutMutexThreadStartArg {
    unsigned int threadNumber;
    FILE *inFile;
    FILE *outFile;
};
static void *processWordsWithoutMutexThreadStart(void *argAsVoidPtr);

/**
 * Run CSCI 451 HW9. This launches threads which each read words from the input file and write them to the output file.
 *
 * @param inFilePath The input file.
 * @param outFilePath The output file.
 * @param mode Whether to run in Mutex or NoMutex mode.
 * @param threadCount The number of threads to launch.
 */
void hw9(
    char const * const inFilePath,
    char const * const outFilePath,
    enum HW9Mode const mode,
    unsigned int const threadCount
) {
    guardNotNull(inFilePath, "inFilePath", "hw9");
    guardNotNull(outFilePath, "outFilePath", "hw9");
    guard(threadCount > 0, "hw9: threadCount must be positive");

    FILE * const inFile = safeFopen(inFilePath, "r", "hw9");
    FILE * const outFile = safeFopen(outFilePath, "w", "hw9");

    pthread_mutex_t fileMutex;

    void *threadStartArgs;
    pthread_t * const threadIds = safeMalloc(sizeof *threadIds * threadCount, "hw9");
    if (mode == HW9Mode_Mutex) {
        safeMutexInit(&fileMutex, NULL, "hw9");

        struct ProcessWordsWithMutexThreadStartArg * const mutexThreadStartArgs = (
            safeMalloc(sizeof *mutexThreadStartArgs * threadCount, "hw9")
        );
        threadStartArgs = mutexThreadStartArgs;

        for (size_t i = 0; i < threadCount; i += 1) {
            struct ProcessWordsWithMutexThreadStartArg * const threadStartArgPtr = &mutexThreadStartArgs[i];

            threadStartArgPtr->threadNumber = (unsigned int)i + 1;
            threadStartArgPtr->inFile = inFile;
            threadStartArgPtr->outFile = outFile;
            threadStartArgPtr->fileMutexPtr = &fileMutex;

            threadIds[i] = safePthreadCreate(
                NULL,
                processWordsWithMutexThreadStart,
                threadStartArgPtr,
                "hw9"
            );
        }
    } else { // mode == HW9Mode_NoMutex
        struct ProcessWordsWithoutMutexThreadStartArg * const noMutexThreadStartArgs = (
            safeMalloc(sizeof *noMutexThreadStartArgs * threadCount, "hw9")
        );
        threadStartArgs = noMutexThreadStartArgs;

        for (size_t i = 0; i < threadCount; i += 1) {
            struct ProcessWordsWithoutMutexThreadStartArg * const threadStartArgPtr = &noMutexThreadStartArgs[i];

            threadStartArgPtr->threadNumber = (unsigned int)i + 1;
            threadStartArgPtr->inFile = inFile;
            threadStartArgPtr->outFile = outFile;

            threadIds[i] = safePthreadCreate(
                NULL,
                processWordsWithoutMutexThreadStart,
                threadStartArgPtr,
                "hw9"
            );
        }
    }

    for (size_t i = 0; i < threadCount; i += 1) {
        pthread_t const threadId = threadIds[i];
        safePthreadJoin(threadId, "hw9");
    }

    free(threadStartArgs);
    free(threadIds);

    if (mode == HW9Mode_Mutex) {
        safeMutexDestroy(&fileMutex, "hw9");
    }

    fclose(inFile);
    fclose(outFile);
}

enum HW9Mode HW9Mode_parse(char const * const name) {
    guardNotNull(name, "name", "HW9Mode_parse");

    if (strcmp(name, "mutex") == 0) {
        return HW9Mode_Mutex;
    }
    if (strcmp(name, "nomutex") == 0) {
        return HW9Mode_NoMutex;
    }

    abortWithErrorFmt("HW9Mode_parse: unknown HW9Mode name \"%s\"", name);
    return (enum HW9Mode)-1;
}

static void *processWordsWithMutexThreadStart(void * const argAsVoidPtr) {
    assert(argAsVoidPtr != NULL);
    struct ProcessWordsWithMutexThreadStartArg * const argPtr = argAsVoidPtr;

    while (true) {
        safeMutexLock(argPtr->fileMutexPtr, "hw9 processWordsWithMutexThreadStart");

        char * const word = readFileLine(argPtr->inFile);
        if (word == NULL) {
            safeMutexUnlock(argPtr->fileMutexPtr, "hw9 processWordsWithMutexThreadStart");
            break;
        }

        fprintf(argPtr->outFile, "%s\t%u\n", word, argPtr->threadNumber);
        free(word);

        safeMutexUnlock(argPtr->fileMutexPtr, "hw9 processWordsWithMutexThreadStart");

        nanosleep(&(struct timespec){
            .tv_sec = 0,
            .tv_nsec = randomInt(0, 1000 * 1000 * 1000)
        }, NULL);
    }

    return NULL;
}

static void *processWordsWithoutMutexThreadStart(void * const argAsVoidPtr) {
    assert(argAsVoidPtr != NULL);
    struct ProcessWordsWithoutMutexThreadStartArg * const argPtr = argAsVoidPtr;

    while (true) {
        char * const word = readFileLine(argPtr->inFile);
        if (word == NULL) {
            break;
        }

        fprintf(argPtr->outFile, "%s\t%u\n", word, argPtr->threadNumber);
        free(word);

        nanosleep(&(struct timespec){
            .tv_sec = 0,
            .tv_nsec = randomInt(0, 1000 * 1000 * 1000)
        }, NULL);
    }

    return NULL;
}
