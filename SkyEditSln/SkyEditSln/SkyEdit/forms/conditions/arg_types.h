#pragma once
#include <cstdint>
#include <initializer_list>
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
   const int32_t     value;
   const char* const string;
   //
   ConditionEnumValue(int32_t a, const char* b) : value(a), string(b) {}
};
enum class ConditionArgUnderlyingType {
   none,
   character, // e.g. Axis
   formID,
   float32,
   integer32,
   string,
   alias,
};
class ConditionArgType {
   public:
      using e_underlying = ConditionArgUnderlyingType;
      //
      virtual void toString(const ConditionArgValue& value, std::string& out) const noexcept;
      virtual bool isValidForEnum(const ConditionArgValue& value) const noexcept;
      virtual void getEnumValues(std::vector<ConditionArgValue>& out) const noexcept;
      virtual bool loadValue(ConditionArgValue& out, TESPluginSubrecord& subrecord) const noexcept;
      //
      ConditionArgType* parent = nullptr;
      std::string  name;
      e_underlying underlying = e_underlying::none;
      bool isEnum   = false;
      bool isSigned = false;
      std::vector<ConditionEnumValue> enumValues;
      std::vector<formtype_t> allowedFormTypes;
      //
      ConditionArgType(const char* n, e_underlying u) : name(n), underlying(u) {}
      ConditionArgType(const char* n, e_underlying u, std::initializer_list<ConditionEnumValue> enumValues) : name(n), underlying(u), isEnum(true), enumValues(enumValues) {}
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
class ConditionArgFormType : public ConditionArgType {
   public:
      ConditionArgFormType(const char* n, std::initializer_list<formtype_t> formTypes) : ConditionArgType(n, ConditionArgUnderlyingType::formID) {
         for (auto it = formTypes.begin(); it != formTypes.end(); ++it)
            this->allowedFormTypes.push_back(*it);
      }
};

namespace ConditionArgTypes {
   extern ConditionArgType     None;
   extern ConditionArgFormType ActorBase;
   extern ConditionArgType     AdvanceAction;
   extern ConditionArgType     Alignment;
   extern ConditionArgFormType AssociationType;
   extern ConditionArgType     Axis;
   extern ConditionArgType     CastingSource;
   extern ConditionArgFormType Class;
   extern ConditionArgType     CrimeType;
   extern ConditionArgType     CriticalStage;
   extern ConditionArgFormType EncounterZone;
   extern ConditionArgFormType Faction;
   extern ConditionArgType     Float;
   extern ConditionArgFormType FormList;
   extern ConditionArgFormType Furniture;
   extern ConditionArgFormType Global;
   extern ConditionArgFormType Keyword;
   extern ConditionArgFormType Location;
   extern ConditionArgFormType MagicEffect;
   extern ConditionArgFormType OwnerForm;
   extern ConditionArgFormType Package;
   extern ConditionArgFormType Perk;
   extern ConditionArgFormType Quest;
   extern ConditionArgFormType Race;
   extern ConditionArgFormType Scene;
   extern ConditionArgType     Sex;
   extern ConditionArgFormType Shout;
   extern ConditionArgFormType Spell;
   extern ConditionArgFormType Voicetype;
   extern ConditionArgFormType Weather;
   extern ConditionArgFormType Worldspace;
}