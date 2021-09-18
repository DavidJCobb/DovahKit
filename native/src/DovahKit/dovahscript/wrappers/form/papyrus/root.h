#pragma once
#include "../../base.h"
#include "../../../wrapper.h"
#include "../../../../dovah/forms/components/papyrus.h"

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc papyrus_root = "PapyRoot";
}

namespace dovahscript::wrappers {
   struct papyrus_root : public wrapper_metatable {
      static constexpr const char* superclass_list = { metatable_key };
      static constexpr const char* metatable_key   = "dovah.classes.papyrus_root";
      static constexpr const char* class_name      = "papyrus_root";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L);

      using wrapped_t = dovah::loaded_forms::components::papyrus::script_data;
      static wrapped_t* unwrap(wrapper& w, bool must_be_end);
      static wrapped_t* unwrap(wrapper& w, bool must_be_end, uint8_t& next_depth);
   };
}