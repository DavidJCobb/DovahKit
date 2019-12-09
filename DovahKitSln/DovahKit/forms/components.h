#pragma once
#include <string>
#include "../helpers/vector3.h"

class TESPluginFile;

struct LStringRef { // a string that may have its value taken from a "STRINGS" string table file.
   uint32_t    index = 0;
   std::string value;
   bool        exists = false; // false if the subrecord wasn't present in the containing form
   //
   // TODO: We'll need to do one of the following:
   //
   //  - Add something to this struct to indicate whether we're pulling from a string table.
   //
   //  - Anyplace we want to be able to edit the value of an LString, we first need to check 
   //    whether the containing file uses a string table.
   //
   // Actually editing string tables is not in-scope for this program, so in cases where a 
   // string table is in use, LStrings should be considered read-only.
   //

   inline const char* c_str() { return this->value.c_str(); }
   inline size_t size() { return this->value.size(); }
};