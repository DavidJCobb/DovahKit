#pragma once
#include "../shout.h"

#include "../../../../dovah/forms/Shout.h"

namespace editor_script::wrappers {
   struct shout_word : public wrapper_metatable {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.shout_word";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_t = dovah::loaded_forms::Shout::Word;
      static wrapped_t* unwrap(wrapper& w);
   };
}