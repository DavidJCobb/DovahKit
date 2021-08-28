#pragma once
#include "../texture_set.h"

namespace dovahscript::wrappers {
   struct texture_set_path_list : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.texture_set_path_list";
      static constexpr const char*   class_name      = "texture_set_path_list";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = texture_set::wrapped_type;
   };
}