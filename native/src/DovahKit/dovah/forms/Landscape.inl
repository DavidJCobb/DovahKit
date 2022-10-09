#pragma once
#include "Landscape.h"

namespace dovah::loaded_forms {
   /*static*/ constexpr void Landscape::quad_offset_to_cell_coords(uint8_t quad, uint8_t index, uint8_t& x, uint8_t& y) {
      x = index % vertices_per_quad_side;
      y = index / vertices_per_quad_side;
      quad_coords_to_cell_coords(quad, x, y);
   }
   /*static*/ constexpr void Landscape::quad_coords_to_cell_coords(uint8_t quad, uint8_t& x, uint8_t& y) {
      switch (quad) {
         case Landscape::quad_indices::bottom_left:
            break;
         case Landscape::quad_indices::top_left:
            y += vertices_per_quad_side - 1;
            break;
         case Landscape::quad_indices::bottom_right:
            x += vertices_per_quad_side - 1;
            break;
         case Landscape::quad_indices::top_right:
            x += vertices_per_quad_side - 1;
            y += vertices_per_quad_side - 1;
            break;
      }
   }
   /*static*/ constexpr bool Landscape::quad_contains_cell_coords(uint8_t quad, uint8_t x, uint8_t y) {
      switch (quad) {
         case quad_indices::bottom_left:
            if (x >= 17)
               return false;
            if (y >= 17)
               return false;
            return true;
         case quad_indices::top_left:
            if (x >= 17)
               return false;
            if (y < 17 || y >= 33)
               return false;
            return true;
         case quad_indices::bottom_right:
            if (x < 17 || x >= 33)
               return false;
            if (y >= 17)
               return false;
            return true;
         case quad_indices::top_right:
            if (x < 17 || x >= 33)
               return false;
            if (y < 17 || y >= 33)
               return false;
            return true;
      }
      return false;
   }
   /*static*/ constexpr void Landscape::cell_coords_to_quad_coords(uint8_t quad, int8_t& x, int8_t& y) {
      if (!quad_contains_cell_coords(quad, x, y)) {
         x = -1;
         y = -1;
         return;
      }
      switch (quad) {
         case quad_indices::bottom_left:
            return;
         case quad_indices::top_left:
            y -= 17;
            return;
         case quad_indices::bottom_right:
            x -= 17;
            return;
         case quad_indices::top_right:
            x -= 17;
            y -= 17;
            return;
      }
   }
}