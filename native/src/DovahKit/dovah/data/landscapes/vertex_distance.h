#pragma once
#include "./vertices_per_cell_side.h"
#include "../../core_constants/exterior_cell_side_length.h"

namespace dovah::landscapes {
   constexpr const size_t vertex_distance = (core_constants::exterior_cell_side_length) / (vertices_per_cell_side - 1);
}
