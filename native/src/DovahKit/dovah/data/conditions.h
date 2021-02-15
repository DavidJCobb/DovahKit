#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "../core.h"
#include "../files/tes_file_reading/elements.h"
#include "../files/tes_file_writing/elements.h"

namespace dovah::loaded_forms::components {
   class condition_parameter;
   class condition;
}

namespace dovah {
   enum class condition_parameter_underlying_type {
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

   class condition_parameter_type {
      public:
         using underlying_t = condition_parameter_underlying_type;
         struct enum_entry {
            int32_t     value = 0;
            const char* name  = "";
            //
            enum_entry(int i, const char* n) : value(int32_t(i)), name(n) {}
            enum_entry(uint32_t i, const char* n) : value(i), name(n) {}
            static_assert(uint32_t(-1) == 0xFFFFFFFF, "The misc stat enum uses unsigned integers, so unsigned-to-signed needs to be bit-identical.");
         };
         //
      protected:
         using condition_parameter      = loaded_forms::components::condition_parameter;
         using union_decider_function_t = condition_parameter_type*(*)(const condition_parameter& decider_value);
         //
      public:
         std::string  name;
         underlying_t underlying = underlying_t::none;
         bool is_enum  = false;
         bool allow_overrides = false; // is this type subject to the "Use Aliases" and "Use Package Data" flags?
         struct {
            condition_parameter_type* type = nullptr;
            union_decider_function_t  func = nullptr;
         } union_decider;
         std::vector<enum_entry>  enum_values;
         std::vector<form_type_t> allowed_form_types;
         
         condition_parameter_type(const char* n, underlying_t u) : name(n), underlying(u) {}
         condition_parameter_type(const char* n, underlying_t u, std::initializer_list<enum_entry> ev) : name(n), underlying(u), is_enum(true), enum_values(ev) {}
         
         bool allows_form_type(form_type_t ft) const noexcept;

         // If the type is flagged as a union, use the next function to get its value; if it returns (nullptr), then the argument "doesn't exist."
         condition_parameter_type* resolve_union(condition_parameter_type* previous_type, const condition_parameter& previous_value) const noexcept;

         inline bool is_none() const noexcept { return this->underlying == underlying_t::none; }
         inline bool is_union() const noexcept { return this->union_decider.type != nullptr; }

         #pragma region Quick constructors
         static condition_parameter_type make_enum_type(const char* n, int32_t first, std::initializer_list<const char*> value_names) {
            condition_parameter_type t(n, underlying_t::int_signed);
            t.is_enum = true;
            int32_t i = first;
            for (auto* n : value_names)
               t.enum_values.emplace_back(i, n);
            return t;
         }
         static condition_parameter_type make_enum_type(const char* n, std::initializer_list<enum_entry> values) {
            condition_parameter_type t(n, underlying_t::int_signed);
            t.is_enum = true;
            t.enum_values.reserve(values.size());
            for (auto& v : values)
               t.enum_values.push_back(v);
            return t;
         }
         static condition_parameter_type make_form_type(const char* n, std::initializer_list<form_type_t> ft, bool allow_overrides = false) {
            condition_parameter_type t(n, underlying_t::formID);
            t.allow_overrides = allow_overrides;
            t.allowed_form_types.reserve(ft.size());
            for (auto v : ft)
               t.allowed_form_types.push_back(v);
            return t;
         }
         static condition_parameter_type make_union_type(const char* n, condition_parameter_type& dt, union_decider_function_t df) {
            condition_parameter_type t(n, underlying_t::int_signed);
            t.union_decider.type = &dt;
            t.union_decider.func = df;
            return t;
         }
         #pragma endregion
   };

   namespace condition_parameter_types {
      extern condition_parameter_type      None; // already defined
      extern condition_parameter_type  Actor;
      extern condition_parameter_type  ActorBase;
      extern condition_parameter_type ActorValue;
      extern condition_parameter_type      AdvanceAction;
      extern condition_parameter_type      Alias; // though some conditions require a ref alias or a loc alias specifically, the CK makes no attempt to ensure you are providing an alias of the correct type
      extern condition_parameter_type      Alignment;
      extern condition_parameter_type  AssociationType;
      extern condition_parameter_type      Axis;
      extern condition_parameter_type  BaseForm; // also includes a few other form types even though GetIsID et. al could never run on them; probably a mistake on Beth's part
      extern condition_parameter_type      CastingSource;
      extern condition_parameter_type  Cell; // testing in CK indicates that only interiors are allowed; named exteriors are not
      extern condition_parameter_type  Class;
      extern condition_parameter_type      CrimeType;
      extern condition_parameter_type      CriticalStage;
      extern condition_parameter_type  EffectItem; // SPEL, ENCH, ALCH, etc.
      extern condition_parameter_type  EncounterZone;
      extern condition_parameter_type      EquipType; // this enum was removed from the game and is only used in one condition, which is both deprecated and broken in two different ways. the CK shows an empty drop-down when trying to choose a value.
      extern condition_parameter_type      Event;
      extern condition_parameter_type  EventData;
      extern condition_parameter_type  Faction;
      extern condition_parameter_type      Float;
      extern condition_parameter_type  FormList;
      extern condition_parameter_type FormType;
      extern condition_parameter_type  Furniture; // TODO: xEdit defs say this can also take a FLST; double-check that and implement if so
      extern condition_parameter_type      FurnitureAnim;
      extern condition_parameter_type      FurnitureEntry;
      extern condition_parameter_type  Global;
      extern condition_parameter_type  Idle;
      extern condition_parameter_type      Integer;
      extern condition_parameter_type  InventoryItem;
      extern condition_parameter_type  Keyword;
      extern condition_parameter_type  KnowableForm; // TODO: Reverse-engineer conditions that use this; if they're not strict about form type, we don't need to be either, since the "Is Known" flag is common to all form types IIRC
      extern condition_parameter_type  Location;
      extern condition_parameter_type  LocRefType;
      extern condition_parameter_type  MagicEffect;
      extern condition_parameter_type      MiscStat; // the values of this enum are CRCs of misc stat name strings
      extern condition_parameter_type  ObjectReference;
      extern condition_parameter_type  OwnerForm;
      extern condition_parameter_type  Package;
      extern condition_parameter_type      PackageData;
      extern condition_parameter_type  Perk;
      extern condition_parameter_type  Quest;
      extern condition_parameter_type      QuestStage;
      extern condition_parameter_type  Race;
      extern condition_parameter_type  Region;
      extern condition_parameter_type  Scene;
      extern condition_parameter_type      Sex;
      extern condition_parameter_type  Shout;
      extern condition_parameter_type  Spell;
      extern condition_parameter_type      String;
      extern condition_parameter_type VATSValue;
      extern condition_parameter_type      VATSValueFunction;
      extern condition_parameter_type  Voicetype; // TODO: xEdit defs say this can also take a FLST; double-check that and implement if so
      extern condition_parameter_type      WardState;
      extern condition_parameter_type  Weather;
      extern condition_parameter_type  Worldspace;
   }

   class condition_function {
      protected:
         enum class _sentinel_is_event {}; // dummy class, for constructor args
         enum class _sentinel_is_dummy {};
         //
      public:
         static constexpr _sentinel_is_event function_uses_event_data = _sentinel_is_event();
         static constexpr _sentinel_is_dummy dummy = _sentinel_is_dummy();
         //
         uint16_t    id = 0xFFFF;
         const char* name = "";
         const char* description = "";
         const bool  valid = true;
         const bool  uses_event_data = false;
         std::array<condition_parameter_type* const, 2> argument_types = { &condition_parameter_types::None, &condition_parameter_types::None }; // aRrAy Of ReFeReNcE iS nOt AlLoWeD
         //
         condition_function() {}; // needed for std::array, apparently
         condition_function(uint16_t id, const char* name, const char* d) : id(id), name(name), description(d) {};
         condition_function(uint16_t id, const char* name, const char* d, condition_parameter_type& a) : id(id), name(name), description(d), argument_types{ &a, &condition_parameter_types::None } {};
         condition_function(uint16_t id, const char* name, const char* d, condition_parameter_type& a, condition_parameter_type& b) : id(id), name(name), description(d), argument_types{ &a, &b } {};
         //
         condition_function(uint16_t id, const char* name, const char* d, _sentinel_is_event) : id(id), name(name), description(d), uses_event_data(true) {};
         //
         condition_function(uint16_t id, _sentinel_is_dummy) : valid(false), id(id), name("Invalid Condition Function"), description("This condition ID is not valid.") {}

         static const condition_function* lookup_by_id(uint16_t) noexcept;
   };

   //
   // This is a list of all ObScript functions in Skyrim. Note that that isn't just 
   // conditions; there are dummy entries for actions as well. When iterating over 
   // the list, check the (valid) field on each entry, or just use the convenience 
   // function provided.
   //
   extern std::array<const condition_function, 736> condition_function_list;
   extern std::array<const condition_function, 5>   extended_condition_function_list; // SKSE additions
   extern bool for_each_condition_function(std::function<bool(const condition_function&)>, bool include_skse = true); // checks (condition_function::valid) for you. return true to stop looping early. function returns whatever the functor did

   struct condition_event_function {
      condition_event_function() = delete;
      enum type : uint16_t {
         GetIsID,
         IsInList,
         GetValue,
         HasKeyword,
         GetItemValue,
      };
   };
}