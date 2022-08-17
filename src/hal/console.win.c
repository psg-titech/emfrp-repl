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

const DWORD START_SIZE = 256;
const DWORD ADD_SIZE = 128;

em_result
read_line(string_t * recycle_buffer) {
    printf("emfrp: ");
    if (recycle_buffer->length == 0) {
        recycle_buffer->buffer = em_malloc(sizeof(char_t) * START_SIZE);
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
        if (ReadConsole(console_input, buf, toRead, &readChars, NULL) == 0)
            return EM_RESULT_UNKNOWN_ERR;
        readed += readChars;
        if (toRead == readChars) {
            char_t * newBuf = em_reallocarray(recycle_buffer->buffer, sizeof(char_t), recycle_buffer->length + ADD_SIZE);
            if (newBuf == nullptr)
                return EM_RESULT_OUT_OF_MEMORY;
            buf = recycle_buffer->buffer + recycle_buffer->length;
            recycle_buffer->length += ADD_SIZE;
            toRead = ADD_SIZE;
        } else break;
    }
    recycle_buffer->buffer = em_reallocarray(recycle_buffer->buffer, sizeof(char_t), readed + 1);
    recycle_buffer->length = readed;
    recycle_buffer->buffer[readed] = '\0';
    return EM_RESULT_OK;
}

#endif

