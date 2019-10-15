#include "output.h"
#include <cstdarg>
#include <stdlib.h>

void _DEBUGMSG(const char* fmt, ...) {
   va_list args;
   va_start(args, fmt);
   vprintf(fmt, args);
   va_end(args);
   printf("\n");
}
const char* FMT_SIGNATURE(uint32_t signature) {
   static char buf[5];
   *(uint32_t*)buf = _byteswap_ulong(signature);
   buf[4] = '\0';
   return buf;
}