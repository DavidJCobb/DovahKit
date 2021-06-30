#pragma once
#include "../worldspace.h"

#include "../../../../dovah/forms/Worldspace.h"

namespace editor_script::wrappers {
   struct worldspace_grid_bounds_extent : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.worldspace_grid_bounds_extent";
      static constexpr const char* class_name     = "worldspace_grid_bounds_extent";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_t = dovah::loaded_forms::Worldspace;
   };
}