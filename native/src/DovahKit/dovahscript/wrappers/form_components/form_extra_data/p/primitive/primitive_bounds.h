#pragma once
#include "dovahscript/wrappers/base.h"
#include "dovahscript/lua_classes/vector3.h"
namespace dovahscript {
   class wrapper;
}

namespace dovahscript::wrappers::form_extra_data_types {
   struct primitive__bounds : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key, dovahscript::lua_classes::vector3::metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.form_extra_data_primitive.bounds";
      static constexpr const char*   class_name      = "form_extra_data_primitive__bounds";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;
   };
}