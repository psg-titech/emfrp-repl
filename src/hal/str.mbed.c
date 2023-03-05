/** -------------------------------------------
 * @file   str.mbed.c
 * @brief  str pollyfill functions for mbed
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/3/3
 ------------------------------------------- */
#if defined(__MBED__)
#include "str.mbed.h"
#include "stdlib.h"

char *strdup(const char * s) {
    unsigned int len = strlen(s);
    char * ret = (char *)malloc(sizeof(char) * (len + 1));
    memcpy(ret, s, len);
    ret[len+1] = '\0';
    return ret;
}
#endif