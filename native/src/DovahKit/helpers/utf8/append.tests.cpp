#include "./append.h"
#include <string>

static_assert([]() -> bool {
   std::string test = "";
   cobb::utf8::append(test, 0xD800);

   if (test[0] != 0b11101101)
      return false;
   if (test[1] != 0b10100000)
      return false;
   if (test[2] != 0b10000000)
      return false;

   return true;
}());

static_assert([]() -> bool {
   std::string test = "";
   cobb::utf8::append(test, 0xFFFD);

   if (test[0] != 0b11101111)
      return false;
   if (test[1] != 0b10111111)
      return false;
   if (test[2] != 0b10111101)
      return false;

   return true;
}());