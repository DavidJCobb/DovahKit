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
      bool isSigned = false; // TODO: replace with two underlying types, "int_signed" and "int_unsigned," and get rid of "integer32"
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
   //
   // TODO:
   //
   //  - Before adding reference types (Actor, etc.), double-check whether their 
   //    values need to be persistent.
   //
   //  - Before adding a Cell type, double-check whether exterior cells are allowed 
   //    (and whether they're required to have editor IDs or be in the default 
   //    worldspace, 0000003C Tamriel).
   //
   //  - Actor
   //  - ActorValue
   //  - Alias
   //     - xEdit allows you to use an alias of any type, but some functions are 
   //       meant only for ref aliases or only for loc aliases. Can we have a type 
   //       for each, to show warnings in UI if the user picks a wrong alias?
   //        - Wait, does the CK allow wrong aliases?
   //  - BaseForm
   //  - Cell
   //  - EquipType
   //  - FurnitureEntryType
   //  - InventoryItem
   //  - MiscStat
   //  - ObjectReference
   //  - PackageData
   //  - QuestStage
   //  - VATSValue
   //     - Any of:
   //        - Weapon (WEAP)
   //        - Weapon List (FLST)
   //        - Target (NPC_)
   //        - Target List (FLST)
   //        - Target Part (actor value index)
   //        - VATS Action
   //             0 = Unarmed
   //             1 = One-Handed Melee
   //             2 = Two-Handed Melee
   //             3 = Magic
   //             4 = Ranged
   //             5 = Reload
   //             6 = Crouch
   //             7 = Stand
   //             8 = Switch Weapon
   //             9 = Draw/Sheathe Weapon
   //             10 = Heal
   //             11 = Player Death
   //        - Critical Effect (SPEL)
   //        - Critical Effect List (FLST)
   //        - Weapon Type (weapon anim type)
   //        - Projectile Type
   //             0 = Missile
   //             1 = Lobber (grenade)
   //             2 = Beam
   //             3 = Flame
   //             4 = Cone
   //             5 = Barrier
   //             6 = Arrow
   //        - Delivery Type
   //        - Casting Type
   //
   //  - Anything related to script variables or animation variables as params; 
   //    how are those handled under the hood?
   //
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
   extern ConditionArgFormType EffectItem; // SPEL, ENCH, ALCH, etc.
   extern ConditionArgFormType EncounterZone;
   extern ConditionArgFormType Faction;
   extern ConditionArgType     Float;
   extern ConditionArgFormType FormList;
   extern ConditionArgFormType Furniture; // TODO: xEdit defs say this can also take a FLST; double-check that and implement if so
   extern ConditionArgType     FurnitureAnim;
   extern ConditionArgType     FurnitureEntry;
   extern ConditionArgFormType Global;
   extern ConditionArgFormType Idle;
   extern ConditionArgType     Integer;
   extern ConditionArgFormType Keyword;
   extern ConditionArgFormType KnowableForm; // TODO: Reverse-engineer conditions that use this; if they're not strict about form type, we don't need to be either, since the "Is Known" flag is common to all form types IIRC
   extern ConditionArgFormType Location;
   extern ConditionArgFormType LocRefType;
   extern ConditionArgFormType MagicEffect;
   extern ConditionArgFormType OwnerForm;
   extern ConditionArgFormType Package;
   extern ConditionArgFormType Perk;
   extern ConditionArgFormType Quest;
   extern ConditionArgFormType Race;
   extern ConditionArgFormType Region;
   extern ConditionArgFormType Scene;
   extern ConditionArgType     Sex;
   extern ConditionArgFormType Shout;
   extern ConditionArgFormType Spell;
   extern ConditionArgType     VATSValueFunction;
   extern ConditionArgFormType Voicetype; // TODO: xEdit defs say this can also take a FLST; double-check that and implement if so
   extern ConditionArgType     WardState;
   extern ConditionArgFormType Weather;
   extern ConditionArgFormType Worldspace;
}