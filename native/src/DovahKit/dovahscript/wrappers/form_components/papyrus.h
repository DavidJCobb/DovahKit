#pragma once
#include "helpers/eight_cc.h"
#include "../base.h"
namespace dovah {
   namespace loaded_forms {
      namespace components::papyrus {
         class attachment_data;
      }
      class Alias;
   }
   class form_stub;
}
namespace dovahscript {
   class wrapper;
}

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc papyrus_root           = "PapyRoot";
   inline constexpr cobb::eight_cc papyrus_script         = "PapyScpt";
   inline constexpr cobb::eight_cc papyrus_property       = "PapyProp";
   inline constexpr cobb::eight_cc papyrus_property_array = "PapyPrpA"; // elements in an array-property's value
   inline constexpr cobb::eight_cc papyrus_frags_named    = "PapyFrag";
   inline constexpr cobb::eight_cc papyrus_frag_begin     = "PapyFBgn"; // INFO, PACK, SCEN
   inline constexpr cobb::eight_cc papyrus_frag_change    = "PapyFChg"; // PACK
   inline constexpr cobb::eight_cc papyrus_frag_end       = "PapyFEnd"; // INFO, PACK, SCEN
   inline constexpr cobb::eight_cc papyrus_frag_indexed   = "PapyFIdx"; // PERK
}
namespace dovahscript::wrappers {
   struct papyrus_root : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.papyrus";
      static constexpr const char*   class_name      = "papyrus";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::components::papyrus::attachment_data;

      static int wrap_and_push(lua_State*, dovah::form_stub&);
      static int wrap_and_push(lua_State*, dovah::loaded_forms::Alias&);
      static wrapped_type* unwrap(wrapper&);
   };
}