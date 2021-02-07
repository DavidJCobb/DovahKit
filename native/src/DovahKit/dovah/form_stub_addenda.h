#pragma once
#include <cstdint>
#include <type_traits>
#include <vector>

namespace dovah {
   class form_stub;

   struct form_stub_addenda {
      struct flag {
         flag() = delete;
         enum type : uint8_t {
            none = 0,
            has_grid_coordinates = 0x01,
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;

      flags_t flags = flag::none;
      //
      struct {
         int32_t x = 0;
         int32_t y = 0;
      } grid_coords; // WRLD/CELL/XCLC
      std::vector<form_stub*> ordered_children; // DIAL/INFO
      form_stub* persistent_cell = nullptr; // WRLD persistent cell

      void sever_references_to(form_stub&);
   };
}
