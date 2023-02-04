/** -------------------------------------------
 * @file   emfrp.c
 * @brief  Console Implementation for Windows
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/18
 ------------------------------------------- */
#if defined(_WIN32)
#include "hal/console.h"
#include "string_t.h"
#include "emmem.h"
#include <stdio.h>
#include <Windows.h>

static HANDLE console_input;

void
initialize_console(void) {
    console_input = GetStdHandle(STD_INPUT_HANDLE);
}

const DWORD START_SIZE = 64;
const DWORD ADD_SIZE = 64;

em_result
read_line(string_t * recycle_buffer) {
    em_result errres = EM_RESULT_OK;
    printf("emfrp: ");
    if (recycle_buffer->length == 0) {
        CHKERR(em_malloc(&(recycle_buffer->buffer), sizeof(char_t) * START_SIZE));
        recycle_buffer->length = START_SIZE;
        if (recycle_buffer->buffer == nullptr)
            return EM_RESULT_OUT_OF_MEMORY;
        recycle_buffer->buffer[0] = '\0';
    }
    if (recycle_buffer->buffer == nullptr)
        return EM_RESULT_INVALID_ARGUMENT;
    DWORD readed = 0;
    DWORD toRead = (DWORD)recycle_buffer->length;
    char_t * buf = recycle_buffer->buffer;
    while (1) {
        DWORD readChars;
        if (ReadConsole(console_input, &buf[readed], toRead, &readChars, NULL) == 0)
            return EM_RESULT_UNKNOWN_ERR;
        bool hasNewLine = false;
        for (int i = readed; i < readChars + readed; ++i)
            hasNewLine |= buf[i] == '\r';
        readed += readChars;
        if (hasNewLine) break;
        char_t * newBuf = nullptr;
        CHKERR(em_reallocarray(&newBuf, recycle_buffer->buffer, sizeof(char_t), recycle_buffer->length + ADD_SIZE));
        recycle_buffer->length += ADD_SIZE;
        recycle_buffer->buffer = newBuf;
        buf = newBuf;
        toRead = ADD_SIZE;
    }
    CHKERR(em_reallocarray(&(recycle_buffer->buffer), recycle_buffer->buffer, sizeof(char_t), readed + 1));
    recycle_buffer->length = readed;
    recycle_buffer->buffer[readed] = '\0';
    return EM_RESULT_OK;
err:
    return errres;
}

#endif

