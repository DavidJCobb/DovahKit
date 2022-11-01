#pragma once
#include <glm/glm.hpp>

#include "./scene_entities/base.h"

namespace vulkanDK {
   class rendered_bounds : public scene_entities::base {
      public:
         static constexpr const char* name_single = "bound";
         static constexpr const char* name_plural = "bounds";
         //
         static constexpr const bool is_drawn = true;
      public:
         struct frame_drawing_data_type { // pass to the shader via a storage buffer
            alignas(16) glm::mat4 transform;    // centerpoint position, rotation, and box size
            alignas(16) glm::vec3 pivot_offset; // pivot's offset from the centerpoint
         };

         frame_drawing_data_type frame_drawing_data;

      protected:
         glm::vec3 _min;
         glm::vec3 _max;
         glm::mat4 _pivot_transform;

      public:
         void mark_for_delete();
         void reset();

         void set_size(const glm::vec3& min, const glm::vec3& max);
         void set_size_and_transform(const glm::vec3& min, const glm::vec3& max, const glm::mat4& pivot_transform);
   };
}