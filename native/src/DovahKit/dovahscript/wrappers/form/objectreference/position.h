#pragma once
#include "../objectreference.h"
#include "../../../lua_classes/vector3.h"

namespace dovahscript::wrappers {
   struct objectreference_position : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key, dovahscript::lua_classes::vector3::metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.objectreference_position";
      static constexpr const char*   class_name      = "objectreference_position";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;
   };
}