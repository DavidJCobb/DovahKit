#pragma once
#include <cstdint>
#include <stdio.h>

extern void _DEBUGMSG(const char* fmt, ...);
extern const char* FMT_SIGNATURE(uint32_t signature); // 'ABCD' -> "ABCD"