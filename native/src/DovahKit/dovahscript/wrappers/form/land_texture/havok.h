#pragma once
#include "../topic_info.h"

namespace dovahscript::wrappers {
   struct land_texture_havok : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.land_texture_havok";
      static constexpr const char*   class_name      = "land_texture_havok";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;
   };
}