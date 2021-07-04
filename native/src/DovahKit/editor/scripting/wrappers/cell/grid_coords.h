#pragma once
#include "../cell.h"

namespace dovah::loaded_forms {
   class Cell;
}
namespace editor_script::wrappers {
   struct cell_grid_coords : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.cell_grid_coords";
      static constexpr const char* class_name     = "cell_grid_coords";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_t = dovah::loaded_forms::Cell;
   };
}