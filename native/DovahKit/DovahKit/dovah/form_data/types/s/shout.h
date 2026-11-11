#pragma once
#include <array>
#include "dovah/types/managed_form_use.h"
#include "dovah/types/managed_localized_string_use.h"
#include "dovah/types/unmanaged_localized_string_use.h"
#include "../_base.h"

#define DOVAH_FORM_TYPE      shout // `dovah::form_type` enumeration member
#define DOVAH_FORM_CLASSNAME Shout // identifier in `dovah::form_data` namespace

namespace dovah::form_data {
   template<form_data_params Params>
   struct Shout : public _base<Params> {
      #include "dovah/form_data/fragments/pull_type_aliases.inl"

      enum class shout_record_flag { // bit indices
         treat_as_power = 7,
      };

      struct word_type {
         form_use spell;                // SNAM+0x04 -> SPEL
         form_use word_of_power;        // SNAM+0x00 -> WOOP
         float    recovery_time = 0.0F; // SNAM+0x08
         
         #define CLASSNAME word_type
         #define TYPENAME  DOVAH_FORM_CLASSNAME<FORM_DATA_PARAMS>::CLASSNAME
         #define PER_FIELD(X) \
            X(spell) \
            X(word_of_power) \
            X(recovery_time)
         #include "dovah/form_data/fragments/define_visitors.inl"
      };
      
      localized_general_string_use     name;                // FULL
      localized_description_string_use description;         // DESC
      form_use                         equip_type;          // ETYP -> EQUP
      form_use                         menu_display_object; // MDOB -> STAT
      std::array<word_type, 3>         words;               // SNAM[3]

      #define CLASSNAME DOVAH_FORM_CLASSNAME
      #define TYPENAME  CLASSNAME<FORM_DATA_PARAMS>
      #define PER_FIELD(X) \
         X(name) \
         X(description) \
         X(equip_type) \
         X(menu_display_object) \
         X(words)
      #include "dovah/form_data/fragments/enable_construction_via_table.inl"
      #include "dovah/form_data/fragments/define_visitors.inl"
   };
}

#include "dovah/form_data/fragments/declare_tabled_load_functions.inl"
#include "dovah/form_data/fragments/define_form_type_constant.inl"

#undef DOVAH_FORM_CLASSNAME
#undef DOVAH_FORM_TYPE