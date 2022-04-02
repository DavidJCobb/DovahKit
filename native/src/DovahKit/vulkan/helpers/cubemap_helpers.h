#pragma once
#include <array>

namespace vulkanDK {
   static constexpr bool cubemaps_are_lefthanded = true;

   // One per cubemap face.
   // These matrices are already inverted and can be used as view matrices directly.
   namespace common_cubemap_views_by_name {
      static constexpr glm::fmat4 x_pos = {
         {  0,  0, -1, 0 },
         {  0, -1,  0, 0 },
         { -1,  0,  0, 0 },
         {  0,  0,  0, 1 },
      };
      static constexpr glm::fmat4 x_neg = {
         {  0,  0,  1, 0 },
         {  0, -1,  0, 0 },
         {  1,  0,  0, 0 },
         {  0,  0,  0, 1 },
      };
      //
      static constexpr glm::fmat4 y_pos = {
         {  1,  0,  0, 0 },
         {  0,  0, -1, 0 },
         {  0,  1,  0, 0 },
         {  0,  0,  0, 1 },
      };
      static constexpr glm::fmat4 y_neg = {
         {  1,  0,  0, 0 },
         {  0,  0,  1, 0 },
         {  0, -1,  0, 0 },
         {  0,  0,  0, 1 },
      };
      //
      static constexpr glm::fmat4 z_pos = {
         {  1,  0,  0, 0 },
         {  0, -1,  0, 0 },
         {  0,  0, -1, 0 },
         {  0,  0,  0, 1 },
      };
      static constexpr glm::fmat4 z_neg = {
         { -1,  0,  0, 0 },
         {  0, -1,  0, 0 },
         {  0,  0,  1, 0 },
         {  0,  0,  0, 1 },
      };
   }

   // One per cubemap face.
   // These matrices are already inverted and can be used as view matrices directly.
   static constexpr std::array<glm::mat4, 6> common_cubemap_views = {
      common_cubemap_views_by_name::x_pos,
      common_cubemap_views_by_name::x_neg,
      common_cubemap_views_by_name::y_pos,
      common_cubemap_views_by_name::y_neg,
      common_cubemap_views_by_name::z_pos,
      common_cubemap_views_by_name::z_neg,
   };
}