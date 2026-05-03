#pragma once
#include <array>
#include <string>
#include "form_data.h"
#include "managed_form_use.h"

// dummy definitions for Compiler Explorer tests
namespace dovah {
   using common_localized_string_use      = std::string;
   using description_localized_string_use = std::string;
}

namespace dovah::form_data {
   template<form_data_params Params>
   struct shout : public _base<Params> {
      #include "define_form_data_type_aliases.inl"

      struct word_type : public type_aliases<Params> {
         form_use spell;
         form_use word_of_power;
         
         #define CLASSNAME word_type
         #define TYPENAME  shout<FORM_DATA_PARAMS>::CLASSNAME
         #define PER_FIELD(X) \
            X(spell) \
            X(word_of_power)
         #include "define_form_data_visitors.inl"
      };
      
      common_localized_string_use      name;                // FULL
      description_localized_string_use description;         // DESC
      form_use                         equip_type;          // ETYP -> EQUP
      form_use                         menu_display_object; // MDOB -> STAT
      std::array<word_type, 3>         words;               // SNAM[3]

      #define CLASSNAME shout
      #define TYPENAME  CLASSNAME<FORM_DATA_PARAMS>
      #define PER_FIELD(X) \
         X(name) \
         X(description) \
         X(equip_type) \
         X(menu_display_object) \
         X(words)
      #include "define_form_data_visitors.inl"
   };
}