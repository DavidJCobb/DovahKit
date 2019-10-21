#pragma once
#include <cstdint>

constexpr int ESP_LOAD_SIMPLE_THREADS     = 4;
constexpr int ESP_LOAD_INT_CELL_THREADS   = 4;
constexpr int ESP_LOAD_WORLDSPACE_THREADS = 6;
constexpr int ESP_LOAD_TOTAL_THREADS = ESP_LOAD_SIMPLE_THREADS + ESP_LOAD_INT_CELL_THREADS + ESP_LOAD_WORLDSPACE_THREADS + 1; // + 1 for the generic complex reader

enum ESPGroupType : int32_t {
   kESPGroupType_FormsOfType = 0,
   kESPGroupType_WorldChildren = 1,
   kESPGroupType_InteriorCellBlock = 2,
   kESPGroupType_InteriorCellSubBlock = 3,
   kESPGroupType_ExteriorCellBlock = 4,
   kESPGroupType_ExteriorCellSubBlock = 5,
   kESPGroupType_CellChildren = 6,
   kESPGroupType_TopicChildren = 7, // DIAL -> INFO
   kESPGroupType_CellPersistentChildren = 8,
   kESPGroupType_CellTemporaryChildren = 9,
};