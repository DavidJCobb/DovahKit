#pragma once
#include "helpers/eight_cc.h"
#include "../../../base.h"
namespace dovah::loaded_forms::components::extra_data_types {
   class primitive;
}
namespace dovahscript {
   class wrapper;
}

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc form_extra_data_primitive_bounds = "PmBounds";
}
namespace dovahscript::wrappers::form_extra_data_types {
   struct primitive : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.form_extra_data_primitive";
      static constexpr const char*   class_name      = "form_extra_data_primitive";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::components::extra_data_types::primitive;
      static constexpr cobb::eight_cc wrapper_part_type = "XtraPrim";

      static wrapped_type* unwrap(wrapper&);
      static void validate_table_for_assign(lua_State* L, int stack_pos);
      static void assign(wrapped_type&, lua_State* L, int stack_pos);
   };
}