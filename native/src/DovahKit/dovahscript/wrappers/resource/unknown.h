#pragma once
#include "../base.h"
#include "../../wrapper.h"

namespace dovahscript::wrappers::resource {
   struct unknown : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.resource.unknown";
      static constexpr const char*   class_name      = "unknown_resource";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;
   };
}