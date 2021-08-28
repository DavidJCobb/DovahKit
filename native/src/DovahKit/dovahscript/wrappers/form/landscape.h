#pragma once
#include "form.h"

namespace dovah::loaded_forms {
   class Landscape;
}

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc landscape_quad        = "LandQuad";
   inline constexpr cobb::eight_cc landscape_alpha_layer = "LandLayr"; // wrapper[LandQuad:3][LandLayr:5]
}

namespace dovahscript::wrappers {
   struct landscape : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.landscape";
      static constexpr const char*   class_name      = "landscape";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Landscape;
   };
}