#pragma once
#include "arg_value.h"
#include <string>
#include <vector>

namespace dovah::loaded_forms::components {
   namespace condition_info {
      /*
      
         There are several different types of condition arguments, such as Actor and Faction, but 
         many of these types are just variations of a much smaller number of underlying types.
      
      */

      enum class arg_underlying_type {
         none,
         aliasID, // the ID of an alias on the quest containing the condition (for conditions outside of quests, it is impossible to specify a valid value)
         character, // e.g. Axis
         event, // dword 0xXXXXYYYY where X is the event member and Y is the event function
         formID,
         float32,
         int_signed,
         int_unsigned,
         package_data, // integer; index of a Package Data in the package containing the condition
         quest_stage, // integer; allowed values depend on what QUST is in the previous argument
         string, // the corresponding integer/dword parameter value will be an integer, but the int's value is random garbage; just use the std::string
      };

      struct enum_value_definition {
         const int32_t     value;
         const char* const string;
         //
         enum_value_definition(int32_t a, const char* b) : value(a), string(b) {}
      };

      class arg_type {
         protected:
            enum class _sentinel_is_union {};
         public:
            static constexpr _sentinel_is_union is_union = _sentinel_is_union();
         public:
            using e_underlying = arg_underlying_type;
            //
            virtual void to_string(const condition_arg_value& value, std::string& out) const noexcept;
            virtual bool is_valid_for_enum(const condition_arg_value& value) const noexcept;
            virtual void get_enum_values(std::vector<condition_arg_value>& out) const noexcept;
            virtual bool load_value(condition_arg_value& out, tes_subrecord_reader& subrecord) const noexcept;
            //
            /// If the type is flagged as a union, use the next function to get its value; if it returns (nullptr), then the argument "doesn't exist."
            virtual arg_type* resolve_union(arg_type* previousType, const condition_arg_value* previousValue) const noexcept;
            //
            arg_type*    parent = nullptr;
            std::string  name;
            e_underlying underlying = e_underlying::none;
            bool         isEnum     = false;
            const bool   isUnion    = false;
            bool         can_be_alias = false;
            std::vector<enum_value_definition> enumValues;
            std::vector<form_type_t> allowedFormTypes;
            //
            arg_type(const char* n, e_underlying u) : name(n), underlying(u) {}
            arg_type(const char* n, e_underlying u, std::initializer_list<enum_value_definition> enumValues) : name(n), underlying(u), isEnum(true), enumValues(enumValues) {}
            arg_type(const char* n, _sentinel_is_union) : name(n), isUnion(true) {}
            //
            bool allows_form_type(form_type_t ft) const noexcept {
               if (this->underlying == arg_underlying_type::formID) {
                  auto& list = this->allowedFormTypes;
                  if (!list.size())
                     return true;
                  for (auto it = list.begin(); it != list.end(); ++it)
                     if (*it == ft)
                        return true;
               }
               return false;
            }
            bool is_none() const noexcept { return this->underlying == e_underlying::none; }
      };
      #pragma region Helper subclasses for quickly instantiating different argument types
      class arg_form_type : public arg_type {
         public:
            arg_form_type(const char* n, std::initializer_list<form_type_t> formTypes, bool can_be_alias = false) : arg_type(n, arg_underlying_type::formID) {
               this->can_be_alias = can_be_alias;
               for (auto it = formTypes.begin(); it != formTypes.end(); ++it)
                  this->allowedFormTypes.push_back(*it);
            }
      };
      class arg_enum_type : public arg_type { // usable for large enum types, when the values are contiguous.
         public:
            arg_enum_type(const char* n, int32_t first, std::initializer_list<const char*> valueNames) : arg_type(n, arg_underlying_type::int_signed) {
               this->isEnum = true;
               int32_t i = first;
               for (auto it = valueNames.begin(); it != valueNames.end(); ++i, ++it)
                  this->enumValues.push_back(enum_value_definition(i, *it));
            }
      };
      class arg_vats_type : public arg_type {
         public:
            arg_vats_type() : arg_type("VATS Value", arg_type::is_union) {};
            //
            virtual arg_type* resolve_union(arg_type* previousType, const condition_arg_value* previousValue) const noexcept override;
      };
      #pragma endregion

      namespace arg_types {
         extern arg_type      None; // already defined
         extern arg_form_type Actor;
         extern arg_form_type ActorBase;
         extern arg_enum_type ActorValue;
         extern arg_type      AdvanceAction;
         extern arg_type      Alias; // though some conditions require a ref alias or a loc alias specifically, the CK makes no attempt to ensure you are providing an alias of the correct type
         extern arg_type      Alignment;
         extern arg_form_type AssociationType;
         extern arg_type      Axis;
         extern arg_form_type BaseForm; // also includes a few other form types even though GetIsID et. al could never run on them; probably a mistake on Beth's part
         extern arg_type      CastingSource;
         extern arg_form_type Cell; // testing in CK indicates that only interiors are allowed; named exteriors are not
         extern arg_form_type Class;
         extern arg_type      CrimeType;
         extern arg_type      CriticalStage;
         extern arg_form_type EffectItem; // SPEL, ENCH, ALCH, etc.
         extern arg_form_type EncounterZone;
         extern arg_type      EquipType; // this enum was removed from the game and is only used in one condition, which is both deprecated and broken in two different ways. the CK shows an empty drop-down when trying to choose a value.
         extern arg_type      Event;
         extern arg_form_type EventData;
         extern arg_form_type Faction;
         extern arg_type      Float;
         extern arg_form_type FormList;
         extern arg_enum_type FormType;
         extern arg_form_type Furniture; // TODO: xEdit defs say this can also take a FLST; double-check that and implement if so
         extern arg_type      FurnitureAnim;
         extern arg_type      FurnitureEntry;
         extern arg_form_type Global;
         extern arg_form_type Idle;
         extern arg_type      Integer;
         extern arg_form_type InventoryItem;
         extern arg_form_type Keyword;
         extern arg_form_type KnowableForm; // TODO: Reverse-engineer conditions that use this; if they're not strict about form type, we don't need to be either, since the "Is Known" flag is common to all form types IIRC
         extern arg_form_type Location;
         extern arg_form_type LocRefType;
         extern arg_form_type MagicEffect;
         extern arg_type      MiscStat; // the values of this enum are CRCs of misc stat name strings
         extern arg_form_type ObjectReference;
         extern arg_form_type OwnerForm;
         extern arg_form_type Package;
         extern arg_type      PackageData;
         extern arg_form_type Perk;
         extern arg_form_type Quest;
         extern arg_type      QuestStage;
         extern arg_form_type Race;
         extern arg_form_type Region;
         extern arg_form_type Scene;
         extern arg_type      Sex;
         extern arg_form_type Shout;
         extern arg_form_type Spell;
         extern arg_type      String;
         extern arg_vats_type VATSValue;
         extern arg_type      VATSValueFunction;
         extern arg_form_type Voicetype; // TODO: xEdit defs say this can also take a FLST; double-check that and implement if so
         extern arg_type      WardState;
         extern arg_form_type Weather;
         extern arg_form_type Worldspace;
      }
   }
}