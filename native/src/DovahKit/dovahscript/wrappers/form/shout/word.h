#pragma once
#include "../shout.h"

#include "../../../../dovah/forms/Shout.h"

namespace dovahscript::wrappers {
   struct shout_word : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.shout_word";
      static constexpr const char*   class_name      = "shout_word";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Shout::Word;
      static wrapped_type* unwrap(wrapper& w);
   };
}