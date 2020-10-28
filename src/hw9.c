#include "../include/hw9.h"

#include "../include/util/memory.h"
#include "../include/util/thread.h"
#include "../include/util/file.h"
#include "../include/util/random.h"
#include "../include/util/guard.h"
#include "../include/util/error.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <assert.h>

struct ProcessWordsWithMutexThreadStartArg {
    FILE *inFile;
    pthread_mutex_t *inFileMutexPtr;
    FILE *outFile;
};
static void *processWordsWithMutexThreadStart(void *argAsVoidPtr);

struct ProcessWordsWithoutMutexThreadStartArg {
    FILE *inFile;
    FILE *outFile;
};
static void *processWordsWithoutMutexThreadStart(void *argAsVoidPtr);

/**
 * Run CSCI 451 HW9. TODO
 */
void hw9(
    char const * const inFilePath,
    char const * const outFilePath,
    enum HW9Mode const mode,
    unsigned int const threadCount
) {
    guardNotNull(inFilePath, "inFilePath", "hw9");
    guardNotNull(outFilePath, "outFilePath", "hw9");
    guard(threadCount > 0, "threadCount must be positive");

    FILE * const inFile = safeFopen(inFilePath, "r", "hw9");
    FILE * const outFile = safeFopen(outFilePath, "w", "hw9");

    pthread_mutex_t inFileMutex;

    void *threadStartArgs;
    pthread_t * const threadIds = safeMalloc(sizeof *threadIds * threadCount, "hw9");
    if (mode == HW9Mode_Mutex) {
        safeMutexInit(&inFileMutex, NULL, "hw9");

        struct ProcessWordsWithMutexThreadStartArg * const mutexThreadStartArgs = (
            safeMalloc(sizeof *mutexThreadStartArgs * threadCount, "hw9")
        );
        threadStartArgs = mutexThreadStartArgs;

        for (size_t i = 0; i < threadCount; i += 1) {
            struct ProcessWordsWithMutexThreadStartArg * const threadStartArgPtr = &mutexThreadStartArgs[i];

            threadStartArgPtr->inFile = inFile;
            threadStartArgPtr->inFileMutexPtr = &inFileMutex;
            threadStartArgPtr->outFile = outFile;

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
        safeMutexDestroy(&inFileMutex, "hw9");
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



    return NULL;
}

static void *processWordsWithoutMutexThreadStart(void * const argAsVoidPtr) {
    assert(argAsVoidPtr != NULL);
    struct ProcessWordsWithoutMutexThreadStartArg * const argPtr = argAsVoidPtr;



    return NULL;
}
