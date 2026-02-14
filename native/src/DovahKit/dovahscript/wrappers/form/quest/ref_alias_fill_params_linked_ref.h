#pragma once
#include "ref_alias.h"
namespace dovah::loaded_forms::structs::alias_fill_params::ref {
   struct find_near_alias;
}

namespace dovahscript::wrappers {
   struct quest_ref_alias_fill_params_linked_ref : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.quest_ref_alias_fill_params_linked_ref";
      static constexpr const char*   class_name      = "quest_ref_alias_fill_params_linked_ref";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::structs::alias_fill_params::ref::find_near_alias;

      static wrapped_type* unwrap(wrapper&);
      static bool validate_table(quest_ref_alias::wrapped_type&, lua_State*, int stack_pos);
      static void overwrite_with_table(quest_ref_alias::wrapped_type&, wrapped_type&, lua_State*, int stack_pos);
   };
}