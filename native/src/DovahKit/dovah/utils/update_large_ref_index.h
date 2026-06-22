#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
#include "../data/game.h"
#include "../forms/components/bounds.h"
#include "../forms/structs/cell_grid_dword.h"
#include "../forms/structs/large_ref_index.h"

namespace dovah {
   namespace loaded_forms {
      class Form;
      class ObjectReference;
      class Worldspace;
   }
   class file_load_order;
   class form_stub;
}

namespace dovah::utils {
   struct update_large_ref_index {
      public:
         using cell_grid_dword = loaded_forms::structs::cell_grid_dword;
         using large_ref_index = loaded_forms::structs::large_ref_index;
         using object_bounds   = loaded_forms::components::object_bounds;

         struct cell_grid_position {
            int16_t y = 0;
            int16_t x = 0;
         };

         struct ref_info {
            form_stub*      ref = nullptr;
            cell_grid_dword parent_cell;
         };

         struct ref_sizing_info {
            float radius;
            cobb::vector3<float> position;
         };

      protected:
         bool _base_form_type_can_ever_be_eligible(dovah::form_type) const;
         bool _loaded_base_form_is_eligible(const dovah::loaded_forms::Form&) const;
         float _base_form_size(form_stub&);
         std::optional<ref_sizing_info> _large_ref_size(form_stub& ref);

      protected:
         form_stub* worldspace = nullptr;
         struct {
            std::unordered_map<form_stub*, float> base_form_sizes; // for non-Static-flagged MSTTs, stores 0
            float large_ref_min_size = 1024.0F; // GMST:fLargeRefMinSize
         } _cache;
      public:
         std::unordered_map<cell_grid_dword, std::vector<ref_info>> cells_to_refs;
         game current_game = game::skyrim_classic;

      protected:
         void _scan_from_worldspace();

      public:
         void gather(form_stub& worldspace);
         void apply(loaded_forms::Form& dst_owner, large_ref_index& dst);
   };
}