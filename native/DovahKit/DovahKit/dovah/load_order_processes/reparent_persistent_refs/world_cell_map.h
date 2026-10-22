#pragma once
#include <vector>
#include "lu/containers/grids/fixed_grid.h"
namespace dovah {
   class form_stub;
}

namespace dovah::load_order_processes::reparent_persistent_refs {
   class world_cell_map {
      protected:
         using cell_list = std::vector<form_stub*>;
         using cell_grid = lu::centered_square_grid<form_stub*, 60>;
      public:
         form_stub* world = nullptr;
         struct {
            cell_grid grid;
            cell_list beyond;
         } cells;
            
         form_stub* cell_for_position(float x, float y) const;
         void set_world(form_stub& world);

         void clear();
   };
}