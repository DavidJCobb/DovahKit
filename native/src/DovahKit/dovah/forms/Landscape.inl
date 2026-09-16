#pragma once
#include "Landscape.h"

namespace dovah::loaded_forms {
   /*static*/ constexpr uint8_t Landscape::cell_relative_vertex_index_to_quad_relative(uint8_t quad, uint16_t ci) {
      int8_t x = ci % vertices_per_side;
      int8_t y = ci / vertices_per_side;
      cell_coords_to_quad_coords(quad, x, y);
      return (uint16_t)x + ((uint16_t)y * vertices_per_quad_side);
   }
   /*static*/ constexpr uint8_t Landscape::quad_relative_vertex_index_to_cell_relative(uint8_t quad, uint16_t qi) {
      uint8_t x = qi % vertices_per_quad_side;
      uint8_t y = qi / vertices_per_quad_side;
      quad_coords_to_cell_coords(quad, x, y);
      return (uint16_t)x + ((uint16_t)y * vertices_per_side);
   }

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
            if (x >= vertices_per_quad_side)
               return false;
            if (y >= vertices_per_quad_side)
               return false;
            return true;
         case quad_indices::top_left:
            if (x >= vertices_per_quad_side)
               return false;
            if (y < vertices_per_quad_side || y >= vertices_per_side)
               return false;
            return true;
         case quad_indices::bottom_right:
            if (x < vertices_per_quad_side || x >= vertices_per_side)
               return false;
            if (y >= vertices_per_quad_side)
               return false;
            return true;
         case quad_indices::top_right:
            if (x < vertices_per_quad_side || x >= vertices_per_side)
               return false;
            if (y < vertices_per_quad_side || y >= vertices_per_side)
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