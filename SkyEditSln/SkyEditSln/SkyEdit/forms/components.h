#pragma once
#include <string>

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

struct Condition {
   enum Operator : uint8_t {
      kOperator_Equal = 0,
      kOperator_NotEqual = 1,
      kOperator_Greater = 2,
      kOperator_GreaterOrEqual = 3,
      kOperator_Less = 4,
      kOperator_LessOrEqual = 5,
   };
   enum RunOn : uint32_t {
      kRunOn_Subject = 0,
      kRunOn_Target = 1,
      kRunOn_Reference = 2,
      kRunOn_CombatTarget = 3,
      kRunOn_LinkedRef = 4,
      kRunOn_Alias = 5,
      kRunOn_PackageData = 6,
      kRunOn_EventData = 7,
   };

   Operator op;
   uint8_t  pad01[3];
   union {
      uint32_t formID;
      float    number;
   } operand;
   uint16_t function;
   uint16_t pad0A;
   struct {
      uint32_t arg1;
      uint32_t arg2;
   } parameters;
   struct {
      uint16_t eventFunc;
      uint16_t eventMember;
      uint32_t formID;
   } eventParams;
   RunOn runOn;
   uint32_t refID; // for Run On Reference
   int32_t unknown = -1;

   void load(TESPluginFile*);
};