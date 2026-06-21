#pragma once
#include "./objectreference.h"

namespace dovahscript::wrappers {
   struct _placed_projectile : public objectreference {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes._placed_projectile";
      static constexpr const char*   class_name      = "_placed_projectile";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr bool has_extra_class_setup = false;
   };
}