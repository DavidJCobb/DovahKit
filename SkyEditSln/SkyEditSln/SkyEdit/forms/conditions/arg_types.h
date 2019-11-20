#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../types.h"

/*

   Experimental code -- not sure if I'm gonna use it yet.

   I'm thinking about making singletons representing every type that can be used 
   for a condition function's arguments, and having the condition function defi-
   nitions just refer to these singletons.

   TODO:

    - Finish writing stringify, etc., functions for these (make a cpp file)

    - Allow these structs to hold a list of allowed form types.

    - Add an inheritance system: allow types to derive from each other so, for 
      example, we can have a single type for "any form" and then have all of 
      the other form types derive from it.

    - When loading form ID values, we need code to convert them from file-local 
      IDs to global IDs (i.e. normalize the load order prefix). This will require 
      us to know what file we're loading them from.

*/

class TESPluginSubrecord;

union ConditionArgValue {
   uint32_t  dword;
   uint8_t   byte;
   float     float32;
   form_id_t formID;
   //
   ConditionArgValue() : dword(0) {}
};
struct ConditionEnumValue {
   int32_t     value;
   const char* string;
};
enum class ConditionArgUnderlyingType {
   none,
   character, // e.g. Axis
   formID,
   float32,
   integer32,
   string,
};
class ConditionArgType {
   public:
      virtual void toString(const ConditionArgValue& value, std::string& out) const noexcept;
      virtual bool isValidForEnum(const ConditionArgValue& value) const noexcept;
      virtual void getEnumValues(std::vector<ConditionArgValue>& out) const noexcept;
      virtual bool loadValue(ConditionArgValue& out, TESPluginSubrecord& subrecord) const noexcept;
      //
      ConditionArgType* parent = nullptr;
      std::string name;
      ConditionArgUnderlyingType underlying = ConditionArgUnderlyingType::none;
      bool isEnum   = false;
      bool isSigned = false;
      std::vector<ConditionEnumValue> enumValues;
      std::vector<formtype_t> allowedFormTypes;
      //
      ConditionArgType(const char* n) : name(n) {}
      //
      bool allowsFormType(formtype_t ft) const noexcept {
         if (this->underlying == ConditionArgUnderlyingType::formID) {
            auto& list = this->allowedFormTypes;
            if (!list.size())
               return true;
            for (auto it = list.begin(); it != list.end(); ++it)
               if (*it == ft)
                  return true;
         }
         return false;
      }
};

namespace ConditionArgTypes {
   class None : public ConditionArgType {
      public:
         None() : ConditionArgType("None") {};
   };
   class _Form : public ConditionArgType {
      public:
         _Form() : ConditionArgType("Form") {
            this->underlying = ConditionArgUnderlyingType::formID;
         };
   };
   class Faction : public _Form {
      public:
         Faction() {
            this->allowedFormTypes.push_back(FormType::Faction);
         }
   };
   class Float : public ConditionArgType {
      public:
         Float() : ConditionArgType("Float") {
            this->underlying = ConditionArgUnderlyingType::float32;
            this->isSigned = true;
         }
         virtual void toString(const ConditionArgValue& value, std::string& out) const noexcept override; // the base function already covers floats, but this will be faster
   };
   class Sex : public ConditionArgType {
      public:
         Sex() : ConditionArgType("Sex") {
            this->underlying = ConditionArgUnderlyingType::integer32;
            this->isEnum = true;
         }
         virtual void toString(const ConditionArgValue& value, std::string& out) const noexcept override;
         virtual bool isValidForEnum(const ConditionArgValue& value) const noexcept override;
         virtual void getEnumValues(std::vector<ConditionArgValue>& out) const noexcept override;
         //
         enum Values { // intentionally unscoped
            male   = 0,
            female = 1,
         };
   };
}