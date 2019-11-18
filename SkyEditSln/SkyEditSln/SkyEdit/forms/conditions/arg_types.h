#pragma once
#include <cstdint>
#include <string>
#include <vector>

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

*/

union ConditionArgValue {
   uint32_t dword;
   uint8_t  byte;
   float    float32;
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
      virtual void toString(const ConditionArgValue& value, std::string& out) const noexcept = 0;
      virtual bool isValidForEnum(const ConditionArgValue& value) const noexcept {
         return false;
      }
      virtual void getEnumValues(std::vector<ConditionArgValue>& out) const noexcept {
         out.clear();
      }
      //
      ConditionArgType* parent = nullptr;
      std::string name;
      ConditionArgUnderlyingType underlying = ConditionArgUnderlyingType::none;
      bool isEnum   = false;
      bool isSigned = false;
      //
      ConditionArgType(const char* n) : name(n) {}
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
         virtual void toString(const ConditionArgValue& value, std::string& out) const noexcept override;
   };
   class Float : public ConditionArgType {
      public:
         Float() : ConditionArgType("Float") {
            this->underlying = ConditionArgUnderlyingType::float32;
            this->isSigned = true;
         }
         virtual void toString(const ConditionArgValue& value, std::string& out) const noexcept override;
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
   };
}