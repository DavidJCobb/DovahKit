#pragma once
#include <optional>
#include "./cell_vertex_coords_to_quad_vertex_coords.h"
#include "./quad_contains_cell_vertex_coords.h"
#include "../../data/landscapes/quad.h"
#include "../../data/landscapes/vertices_per_cell_side.h"
#include "../../data/landscapes/vertices_per_quad_side.h"

namespace dovah::utils::landscapes {
   constexpr std::optional<size_t> cell_vertex_index_to_quad_vertex_index(dovah::landscapes::quad q, size_t ci) {
      using namespace dovah::landscapes;

      std::pair<uint8_t, uint8_t> cc;
      auto& [cx, cy] = cc;
      cx = ci % vertices_per_cell_side;
      cy = ci / vertices_per_cell_side;
      if (!quad_contains_cell_vertex_coords(q, cc))
         return {};
      auto [qx, qy] = cell_vertex_coords_to_quad_vertex_coords(q, cc);
      return qx + (qy * vertices_per_quad_side);
   }
}