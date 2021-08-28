#pragma once
#include "../worldspace.h"

namespace dovah::loaded_forms {
   class Worldspace;
}

namespace dovahscript::wrappers {
   struct worldspace_grid_bounds_extent : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.worldspace_grid_bounds_extent";
      static constexpr const char*   class_name      = "worldspace_grid_bounds_extent";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Worldspace;
   };
}