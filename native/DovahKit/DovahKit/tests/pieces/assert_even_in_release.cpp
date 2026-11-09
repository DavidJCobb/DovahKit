#include "./assert_even_in_release.h"

#ifndef _DEBUG
   static volatile unsigned int stored_line = 0;

   void _____assert_even_in_release_dummy(unsigned int line) {
      stored_line = line;
   }
#endif