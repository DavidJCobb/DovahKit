#pragma once
#include "../cell.h"

namespace dovahscript::wrappers {
   struct cell_grid_coords : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.cell_grid_coords";
      static constexpr const char*   class_name      = "cell_grid_coords";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;
   };
}