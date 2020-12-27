#pragma once
#include "../../wrapper.h"
#include "../../../../dovah/forms/components/papyrus.h"

namespace editor_script::wrappers {
   struct papyrus_script : public wrapper_metatable {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.papyrus_script";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_t = dovah::loaded_forms::components::papyrus::script_data::script;
      static wrapped_t* unwrap(wrapper& w);
   };
}