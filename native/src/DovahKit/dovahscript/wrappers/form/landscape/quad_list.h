#pragma once
#include "../landscape.h"

namespace dovahscript::wrappers {
   //
   // landscape.quads.top_left.default_texture
   // landscape.quads.top_left.layers[1]
   //
   struct landscape_quad_list : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.landscape_quad_list";
      static constexpr const char*   class_name      = "landscape_quad_list";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;
   };
}