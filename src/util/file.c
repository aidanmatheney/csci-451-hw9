#include "../../include/util/file.h"

#include "../../include/util/guard.h"
#include "../../include/util/error.h"

#include "../../include/util/StringBuilder.h"

#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>

/**
 * Open the file using fopen. If the operation fails, abort the program with an error message.
 *
 * @param filePath The file path.
 * @param modes The fopen modes string.
 * @param callerDescription A description of the caller to be included in the error message. This could be the name of
 *                          the calling function, plus extra information if useful.
 *
 * @returns The opened file.
 */
FILE *safeFopen(char const * const filePath, char const * const modes, char const * const callerDescription) {
    guardNotNull(filePath, "filePath", "safeFopen");
    guardNotNull(modes, "modes", "safeFopen");
    guardNotNull(callerDescription, "callerDescription", "safeFopen");

    FILE * const file = fopen(filePath, modes);
    if (file == NULL) {
        int const fopenErrorCode = errno;
        char const * const fopenErrorMessage = strerror(fopenErrorCode);

        abortWithErrorFmt(
            "%s: Failed to open file \"%s\" with modes \"%s\" using fopen (error code: %d; error message: \"%s\")",
            callerDescription,
            filePath,
            modes,
            fopenErrorCode,
            fopenErrorMessage
        );
        return NULL;
    }

    return file;
}

/**
 * Print a formatted string to the given file. If the operation fails, abort the program with an error message.
 *
 * @param file The file.
 * @param callerDescription A description of the caller to be included in the error message. This could be the name of
 *                          the calling function, plus extra information if useful.
 * @param format The format (printf).
 * @param ... The format arguments (printf).
 *
 * @returns The number of charactes printed (no string terminator character).
 */
unsigned int safeFprintf(
    FILE * const file,
    char const * const callerDescription,
    char const * const format,
    ...
) {
    va_list formatArgs;
    va_start(formatArgs, format);
    unsigned int const printedCharCount = safeVfprintf(file, format, formatArgs, callerDescription);
    va_end(formatArgs);
    return printedCharCount;
}

/**
 * Print a formatted string to the given file. If the operation fails, abort the program with an error message.
 *
 * @param file The file.
 * @param format The format (printf).
 * @param formatArgs The format arguments (printf).
 * @param callerDescription A description of the caller to be included in the error message. This could be the name of
 *                          the calling function, plus extra information if useful.
 *
 * @returns The number of charactes printed (no string terminator character).
 */
unsigned int safeVfprintf(
    FILE * const file,
    char const * const format,
    va_list formatArgs,
    char const * const callerDescription
) {
    guardNotNull(file, "file", "safeVfprintf");
    guardNotNull(format, "format", "safeVfprintf");
    guardNotNull(callerDescription, "callerDescription", "safeVfprintf");

    int const printedCharCount = vfprintf(file, format, formatArgs);
    if (printedCharCount < 0) {
        int const vfprintfErrorCode = errno;
        char const * const vfprintfErrorMessage = strerror(vfprintfErrorCode);

        abortWithErrorFmt(
            "%s: Failed to print format \"%s\" to file using vfprintf (error code: %d; error message: \"%s\")",
            callerDescription,
            format,
            vfprintfErrorCode,
            vfprintfErrorMessage
        );
        return (unsigned int)-1;
    }

    return (unsigned int)printedCharCount;
}

/**
 * Read a character from the given file. If the operation fails, abort the program with an error message.
 *
 * @param charPtr The location to store the read character.
 * @param file The file to read from.
 * @param callerDescription A description of the caller to be included in the error message. This could be the name of
 *                          the calling function, plus extra information if useful.
 *
 * @returns Whether the end-of-file was hit.
 */
bool safeFgetc(
    char * const charPtr,
    FILE * const file,
    char const * const callerDescription
) {
    guardNotNull(charPtr, "charPtr", "safeFgetc");
    guardNotNull(file, "file", "safeFgetc");
    guardNotNull(callerDescription, "callerDescription", "safeFgetc");

    int const fgetcResult = fgetc(file);
    if (fgetcResult == EOF) {
        bool const fgetcError = ferror(file);
        if (fgetcError) {
            int const fgetcErrorCode = errno;
            char const * const fgetcErrorMessage = strerror(fgetcErrorCode);

            abortWithErrorFmt(
                "%s: Failed to read char from file using fgetc (error code: %d; error message: \"%s\")",
                callerDescription,
                fgetcErrorCode,
                fgetcErrorMessage
            );
            return false;
        }

        // EOF
        return false;
    }

    *charPtr = (char)fgetcResult;
    return true;
}

/**
 * Read characters from the given file into the given buffer. Stop as soon as one of the following conditions has been
 * met: (A) `bufferLength - 1` characters have been read, (B) a newline is encountered, or (C) the end of the file is
 * reached. The string read into the buffer will end with a terminating character. If the operation fails, abort the
 * program with an error message.
 *
 * @param buffer The buffer into which to read the string.
 * @param bufferLength The length of the buffer.
 * @param file The file to read from.
 * @param callerDescription A description of the caller to be included in the error message. This could be the name of
 *                          the calling function, plus extra information if useful.
 *
 * @returns Whether unread characters remain.
 */
bool safeFgets(
    char * const buffer,
    size_t const bufferLength,
    FILE * const file,
    char const * const callerDescription
) {
    guardNotNull(buffer, "buffer", "safeFgets");
    guardNotNull(file, "file", "safeFgets");
    guardNotNull(callerDescription, "callerDescription", "safeFgets");

    char * const fgetsResult = fgets(buffer, (int)bufferLength, file);
    bool const fgetsError = ferror(file);
    if (fgetsError) {
        int const fgetsErrorCode = errno;
        char const * const fgetsErrorMessage = strerror(fgetsErrorCode);

        abortWithErrorFmt(
            "%s: Failed to read %zu chars from file using fgets (error code: %d; error message: \"%s\")",
            callerDescription,
            bufferLength,
            fgetsErrorCode,
            fgetsErrorMessage
        );
        return false;
    }

    if (fgetsResult == NULL || feof(file)) {
        return false;
    }

    return true;
}

/**
 * Read a line from the file. If the current file position is EOF, return null.
 *
 * @param file The file to read from.
 *
 * @returns The line (the caller is responsible for freeing this memory), or null if the current file position is EOF.
 */
char *readFileLine(FILE * const file) {
    guardNotNull(file, "file", "readFileLine");

    StringBuilder const lineBuilder = StringBuilder_create();

    bool lineBeginsAtEof = true;
    char readCharacter;
    while (safeFgetc(&readCharacter, file, "readFileLine")) {
        lineBeginsAtEof = false;

        if (readCharacter == '\n') {
            break;
        }

        StringBuilder_appendChar(lineBuilder, readCharacter);
    }

    if (lineBeginsAtEof) {
        StringBuilder_destroy(lineBuilder);
        return NULL;
    }

    char * const line = StringBuilder_toStringAndDestroy(lineBuilder);
    return line;
}

/**
 * Open a text file, read all the text in the file into a string, and then close the file.
 *
 * @param filePath The path to the file.
 *
 * @returns A string containing all text in the file. The caller is responsible for freeing this memory.
 */
char *readAllFileText(char const * const filePath) {
    guardNotNull(filePath, "filePath", "readAllFileText");

    StringBuilder const fileTextBuilder = StringBuilder_create();

    FILE * const file = safeFopen(filePath, "r", "readAllFileText");
    char fgetsBuffer[100];
    while (safeFgets(fgetsBuffer, 100, file, "readAllFileText")) {
        StringBuilder_append(fileTextBuilder, fgetsBuffer);
    }
    fclose(file);

    char * const fileText = StringBuilder_toStringAndDestroy(fileTextBuilder);

    return fileText;
}

/**
 * Read values from the given file using the given format. Values are stored in the locations pointed to by formatArgs.
 * If the operation fails, abort the program with an error message.
 *
 * @param file The file.
 * @param callerDescription A description of the caller to be included in the error message. This could be the name of
 *                          the calling function, plus extra information if useful.
 * @param format The format (scanf).
 * @param ... The format arguments (scanf).
 *
 * @returns The number of input items successfully matched and assigned, which can be fewer than provided for, or even
 *          zero in the event of an early matching failure. EOF is returned if the end of input is reached before either
 *          the first successful conversion or a matching failure occurs.
 */
int safeFscanf(
    FILE * const file,
    char const * const callerDescription,
    char const * const format,
    ...
) {
    va_list formatArgs;
    va_start(formatArgs, format);
    int const matchCount = safeVfscanf(file, format, formatArgs, callerDescription);
    va_end(formatArgs);
    return matchCount;
}

/**
 * Read values from the given file using the given format. Values are stored in the locations pointed to by formatArgs.
 * If the operation fails, abort the program with an error message.
 *
 * @param file The file.
 * @param format The format (scanf).
 * @param formatArgs The format arguments (scanf).
 * @param callerDescription A description of the caller to be included in the error message. This could be the name of
 *                          the calling function, plus extra information if useful.
 *
 * @returns The number of input items successfully matched and assigned, which can be fewer than provided for, or even
 *          zero in the event of an early matching failure. EOF is returned if the end of input is reached before either
 *          the first successful conversion or a matching failure occurs.
 */
int safeVfscanf(
    FILE * const file,
    char const * const format,
    va_list formatArgs,
    char const * const callerDescription
) {
    guardNotNull(file, "file", "safeVfscanf");
    guardNotNull(format, "format", "safeVfscanf");
    guardNotNull(callerDescription, "callerDescription", "safeVfscanf");

    int const matchCount = vfscanf(file, format, formatArgs);
    bool const vfscanfError = ferror(file);
    if (vfscanfError) {
        int const vfscanfErrorCode = errno;
        char const * const vfscanfErrorMessage = strerror(vfscanfErrorCode);

        abortWithErrorFmt(
            "%s: Failed to read format \"%s\" from file using vfscanf (error code: %d; error message: \"%s\")",
            callerDescription,
            format,
            vfscanfErrorCode,
            vfscanfErrorMessage
        );
        return -1;
    }

    return matchCount;
}

/**
 * Read values from the given file using the given format. Values are stored in the locations pointed to by formatArgs.
 * If the number of matched items does not match the expected count or if the operation fails, abort the program with an
 * error message.
 *
 * @param file The file.
 * @param expectedMatchCount The number of items in the format expected to be matched.
 * @param format The format (scanf).
 * @param ... The format arguments (scanf).
 *
 * @returns True if the format was scanned, or false if the end of the file was met.
 */
bool scanFileExact(
    FILE * const file,
    unsigned int const expectedMatchCount,
    char const * const format,
    ...
) {
    va_list formatArgs;
    va_start(formatArgs, format);
    bool const scanned = scanFileExactVA(file, expectedMatchCount, format, formatArgs);
    va_end(formatArgs);
    return scanned;
}

/**
 * Read values from the given file using the given format. Values are stored in the locations pointed to by formatArgs.
 * If the number of matched items does not match the expected count or if the operation fails, abort the program with an
 * error message.
 *
 * @param file The file.
 * @param expectedMatchCount The number of items in the format expected to be matched.
 * @param format The format (scanf).
 * @param formatArgs The format arguments (scanf).
 *
 * @returns True if the format was scanned, or false if the end of the file was met.
 */
bool scanFileExactVA(
    FILE * const file,
    unsigned int const expectedMatchCount,
    char const * const format,
    va_list formatArgs
) {
    guardNotNull(file, "file", "scanFileExactVA");
    guardNotNull(format, "format", "scanFileExactVA");

    int const matchCount = safeVfscanf(file, format, formatArgs, "scanFileExactVA");
    if (matchCount == EOF) {
        return false;
    }

    if ((unsigned int)matchCount != expectedMatchCount) {
        abortWithErrorFmt(
            "scanFileExactVA: Failed to parse exact format \"%s\" from file"
            " (expected match count: %u; actual match count: %d)",
            format,
            expectedMatchCount,
            matchCount
        );
        return false;
    }

    return true;
}
