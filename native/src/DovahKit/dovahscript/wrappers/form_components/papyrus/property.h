#pragma once
#include "../condition.h"
namespace dovah::loaded_forms::components::papyrus {
   class property;
}

namespace dovahscript::wrappers {
   struct papyrus_property : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.papyrus_property";
      static constexpr const char*   class_name      = "papyrus_property";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::components::papyrus::property;

      static wrapped_type* unwrap(wrapper&);
   };
}