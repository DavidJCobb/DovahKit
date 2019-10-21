#pragma once
#include <cstdint>
#include <stdio.h>

extern void _DEBUGMSG(const char* fmt, ...); // appends a newline
extern void _DEBUGPRINT(const char* fmt, ...); // does not append a newline
extern const char* FMT_SIGNATURE(uint32_t signature); // 'ABCD' -> "ABCD" // not thread-safe, and can only use one buffer at a time
extern const char* FMT_SIGNATURE(uint32_t signature, char out[5]); // 'ABCD' -> "ABCD" // thread-safe if out buffer is local to the thread