#pragma once
#include "../objectreference.h"
#include "../../../lua_classes/euler.h"

namespace dovahscript::wrappers {
   struct objectreference_rotation : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key, dovahscript::lua_classes::euler::metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.objectreference_rotation";
      static constexpr const char*   class_name      = "objectreference_rotation";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;
   };
}