#pragma once
#include "form.h"

namespace dovah::loaded_forms {
   class Static;
}

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc static_directional_material = "StatDMat";
   inline constexpr cobb::eight_cc static_distant_lod_paths    = "StatDLOD";
}

namespace dovahscript::wrappers {
   struct statik : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.static";
      static constexpr const char*   class_name      = "static";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Static;
   };
}