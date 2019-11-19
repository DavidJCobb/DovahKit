#pragma once
#include <cstdint>
#include "../helpers/scoped_enum.h"

constexpr int ESP_LOAD_SIMPLE_THREADS     = 4;
constexpr int ESP_LOAD_INT_CELL_THREADS   = 4;
constexpr int ESP_LOAD_WORLDSPACE_THREADS = 6;
constexpr int ESP_LOAD_TOTAL_THREADS = ESP_LOAD_SIMPLE_THREADS + ESP_LOAD_INT_CELL_THREADS + ESP_LOAD_WORLDSPACE_THREADS + 1; // + 1 for the generic complex reader

SCOPE_ENUM(ESPGroupType, enum ESPGroupType : int32_t {
   forms_of_type            = 0,
   world_children           = 1,
   interior_cell_block      = 2,
   interior_cell_sub_block  = 3,
   exterior_cell_block      = 4,
   exterior_cell_sub_block  = 5,
   cell_children            = 6,
   topic_children           = 7, SCOPED_ENUM_COMMENT("DIAL -> INFO")
   cell_persistent_children = 8,
   cell_temporary_children  = 9,
});