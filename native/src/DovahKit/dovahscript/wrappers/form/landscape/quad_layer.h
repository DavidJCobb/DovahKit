#pragma once
#include "../landscape.h"

namespace dovahscript::wrappers {
   //
   // landscape.quads.top_left.layers[1].texture
   // landscape.quads.top_left.layers[1].indices[3]
   //
   struct landscape_quad_alpha_layer : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.landscape_quad_alpha_layer";
      static constexpr const char*   class_name      = "landscape_quad_alpha_layer";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;
   };
} 