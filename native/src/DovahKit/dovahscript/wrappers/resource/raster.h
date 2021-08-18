#pragma once
#include "../base.h"
#include "../../wrapper.h"

namespace dovahscript {
   class DovahscriptResource;
}

namespace dovahscript::wrappers::resource {
   struct raster : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key    = "dovah.classes.resource.raster";
      static constexpr const char*   class_name       = "raster";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "raster";

      // Creates a singleton for this class, and leaves it at the top of the stack.
      static void import_singleton(lua_State*);
   };
}