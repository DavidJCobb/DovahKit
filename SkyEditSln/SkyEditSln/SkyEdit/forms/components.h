#pragma once
#include <string>

class TESPluginFile;

struct LStringRef {
   uint32_t    index = 0;
   std::string value;
   bool        exists = false; // false if the subrecord wasn't present in the containing form

   inline const char* c_str() { return this->value.c_str(); }
   inline size_t size() { return this->value.size(); }
};