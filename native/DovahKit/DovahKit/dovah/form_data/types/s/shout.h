#pragma once
#include <array>
#include "lu/passkey.h"
#include "dovah/form_data/helpers/type_aliases.h"
#include "dovah/form_type.h"
#include "dovah/types/managed_form_use.h"
#include "dovah/types/managed_localized_string_use.h"
#include "dovah/types/unmanaged_localized_string_use.h"
#include "../_base.h"
namespace dovah {
   namespace load_order_interfaces {
      class form_load;
   }
   namespace load_order_processes::use_info_build {
      class use_info_builder;
   }
   namespace tes {
      class record_view;
   }
}

namespace dovah::form_data {
   template<form_data_params Params>
   struct shout : public _base<Params> {
      #include "dovah/form_data/helpers/pull_type_aliases.inl"

      enum class shout_record_flag { // bit indices
         treat_as_power = 7,
      };

      struct word_type {
         form_use spell;                // SNAM+0x04 -> SPEL
         form_use word_of_power;        // SNAM+0x00 -> WOOP
         float    recovery_time = 0.0F; // SNAM+0x08
         
         #define CLASSNAME word_type
         #define TYPENAME  shout<FORM_DATA_PARAMS>::CLASSNAME
         #define PER_FIELD(X) \
            X(spell) \
            X(word_of_power) \
            X(recovery_time)
         #include "dovah/form_data/helpers/define_visitors.inl"
      };
      
      localized_general_string_use     name;                // FULL
      localized_description_string_use description;         // DESC
      form_use                         equip_type;          // ETYP -> EQUP
      form_use                         menu_display_object; // MDOB -> STAT
      std::array<word_type, 3>         words;               // SNAM[3]

      #define DOVAH_FORM_TYPE form_type::shout
      #define CLASSNAME shout
      #define TYPENAME  CLASSNAME<FORM_DATA_PARAMS>
      #define PER_FIELD(X) \
         X(name) \
         X(description) \
         X(equip_type) \
         X(menu_display_object) \
         X(words)
      #include "dovah/form_data/helpers/enable_construction_via_table.inl"
      #include "dovah/form_data/helpers/define_visitors.inl"
   };
}

#define DOVAH_FORM_TYPE      form_type::shout
#define DOVAH_FORM_CLASSNAME shout
#include "dovah/form_data/helpers/define_form_type_constant.inl"

namespace dovah::form_data_function_tables::build_outbound_use_info {
   template<form_type>
   struct entry;

   template<>
   struct entry<form_type::shout> {
      static void execute(tes::record_view&, load_order_processes::use_info_build::use_info_builder&);
   };
}
namespace dovah::form_data_function_tables::serialize_form_data {
   extern void load(form_data::shout<managed_form_params>&, tes::record_view&, load_order_interfaces::form_load&);
}