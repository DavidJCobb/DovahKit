#pragma once
#include "../statik.h"

namespace dovah::loaded_forms {
   class Static;
}

namespace dovahscript::wrappers {
   struct static_distant_lod_paths : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.static_distant_lod_paths";
      static constexpr const char*   class_name      = "static_distant_lod_paths";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Static;
   };
}