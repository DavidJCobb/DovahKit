#pragma once
#include "form.h"

namespace dovah::loaded_forms {
   class Cell;
}

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc cell_grid_coords = "CellGrid";
}

namespace dovahscript::wrappers {
   struct cell : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.cell";
      static constexpr const char*   class_name      = "cell";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Cell;
   };
}