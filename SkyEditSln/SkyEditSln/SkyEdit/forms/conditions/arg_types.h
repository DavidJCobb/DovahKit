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
   aliasID, // the ID of an alias on the quest containing the condition (for conditions outside of quests, it is impossible to specify a valid value)
   character, // e.g. Axis
   formID,
   float32,
   int_signed,
   int_unsigned,
   package_data, // integer; index of a Package Data in the package containing the condition
   quest_stage, // integer; allowed values depend on what QUST is in the previous argument
   string, // the corresponding integer/dword parameter value will be an integer, but the int's value is random garbage; just use the std::string
};
class ConditionArgType {
   public:
      //
      // Dummy classes, for constructors:
      //
      enum class sentinel_is_union {};
      static constexpr sentinel_is_union is_union = sentinel_is_union();
   public:
      using e_underlying = ConditionArgUnderlyingType;
      //
      virtual void toString(const ConditionArgValue& value, std::string& out) const noexcept;
      virtual bool isValidForEnum(const ConditionArgValue& value) const noexcept;
      virtual void getEnumValues(std::vector<ConditionArgValue>& out) const noexcept;
      virtual bool loadValue(ConditionArgValue& out, TESPluginSubrecord& subrecord) const noexcept;
      //
      /// If the type is flagged as a union, use the next function to get its value; if it returns (nullptr), then the argument "doesn't exist."
      virtual ConditionArgType* resolveUnion(ConditionArgType* previousType, ConditionArgValue* previousValue) const noexcept;
      //
      ConditionArgType* parent = nullptr;
      std::string  name;
      e_underlying underlying = e_underlying::none;
      bool isEnum = false;
      const bool isUnion = false;
      std::vector<ConditionEnumValue> enumValues;
      std::vector<formtype_t> allowedFormTypes;
      //
      ConditionArgType(const char* n, e_underlying u) : name(n), underlying(u) {}
      ConditionArgType(const char* n, e_underlying u, std::initializer_list<ConditionEnumValue> enumValues) : name(n), underlying(u), isEnum(true), enumValues(enumValues) {}
      ConditionArgType(const char* n, sentinel_is_union) : name(n), isUnion(true) {}
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
   //
   // Helper subclass for quickly instantiating form types.
   //
   public:
      ConditionArgFormType(const char* n, std::initializer_list<formtype_t> formTypes) : ConditionArgType(n, ConditionArgUnderlyingType::formID) {
         for (auto it = formTypes.begin(); it != formTypes.end(); ++it)
            this->allowedFormTypes.push_back(*it);
      }
};
class ConditionArgEnumType : public ConditionArgType {
   //
   // Helper subclass for quickly instantiating large enum types, when the values are contiguous.
   //
   public:
      ConditionArgEnumType(const char* n, int32_t first, std::initializer_list<const char*> valueNames) : ConditionArgType(n, ConditionArgUnderlyingType::int_signed) {
         int32_t i = first;
         for (auto it = valueNames.begin(); it != valueNames.end(); ++i, ++it)
            this->enumValues.push_back(ConditionEnumValue(i, *it));
      }
};

class ConditionVATSValueUnion : public ConditionArgType {
   public:
      ConditionVATSValueUnion() : ConditionArgType("VATS Value", ConditionArgType::is_union) {};
      //
      virtual ConditionArgType* resolveUnion(ConditionArgType* previousType, ConditionArgValue* previousValue) const noexcept override;
};

namespace ConditionArgTypes {
   extern ConditionArgType     None;
   extern ConditionArgFormType Actor;
   extern ConditionArgFormType ActorBase;
   extern ConditionArgEnumType ActorValue;
   extern ConditionArgType     AdvanceAction;
   extern ConditionArgType     Alias; // though some conditions require a ref alias or a loc alias specifically, the CK makes no attempt to ensure you are providing an alias of the correct type
   extern ConditionArgType     Alignment;
   extern ConditionArgFormType AssociationType;
   extern ConditionArgType     Axis;
   extern ConditionArgFormType BaseForm; // also includes a few other form types even though GetIsID et. al could never run on them; probably a mistake on Beth's part
   extern ConditionArgType     CastingSource;
   extern ConditionArgFormType Cell; // testing in CK indicates that only interiors are allowed; named exteriors are not
   extern ConditionArgFormType Class;
   extern ConditionArgType     CrimeType;
   extern ConditionArgType     CriticalStage;
   extern ConditionArgFormType EffectItem; // SPEL, ENCH, ALCH, etc.
   extern ConditionArgFormType EncounterZone;
   extern ConditionArgType     EquipType; // this enum was removed from the game and is only used in one condition, which is both deprecated and broken in two different ways. the CK shows an empty drop-down when trying to choose a value.
   extern ConditionArgFormType Faction;
   extern ConditionArgType     Float;
   extern ConditionArgFormType FormList;
   extern ConditionArgFormType Furniture; // TODO: xEdit defs say this can also take a FLST; double-check that and implement if so
   extern ConditionArgType     FurnitureAnim;
   extern ConditionArgType     FurnitureEntry;
   extern ConditionArgFormType Global;
   extern ConditionArgFormType Idle;
   extern ConditionArgType     Integer;
   extern ConditionArgFormType InventoryItem;
   extern ConditionArgFormType Keyword;
   extern ConditionArgFormType KnowableForm; // TODO: Reverse-engineer conditions that use this; if they're not strict about form type, we don't need to be either, since the "Is Known" flag is common to all form types IIRC
   extern ConditionArgFormType Location;
   extern ConditionArgFormType LocRefType;
   extern ConditionArgFormType MagicEffect;
   extern ConditionArgType     MiscStat; // the values of this enum are CRCs of misc stat name strings
   extern ConditionArgFormType ObjectReference;
   extern ConditionArgFormType OwnerForm;
   extern ConditionArgFormType Package;
   extern ConditionArgType     PackageData;
   extern ConditionArgFormType Perk;
   extern ConditionArgFormType Quest;
   extern ConditionArgType     QuestStage;
   extern ConditionArgFormType Race;
   extern ConditionArgFormType Region;
   extern ConditionArgFormType Scene;
   extern ConditionArgType     Sex;
   extern ConditionArgFormType Shout;
   extern ConditionArgFormType Spell;
   extern ConditionArgType     String;
   extern ConditionVATSValueUnion VATSValue;
   extern ConditionArgType     VATSValueFunction;
   extern ConditionArgFormType Voicetype; // TODO: xEdit defs say this can also take a FLST; double-check that and implement if so
   extern ConditionArgType     WardState;
   extern ConditionArgFormType Weather;
   extern ConditionArgFormType Worldspace;
}