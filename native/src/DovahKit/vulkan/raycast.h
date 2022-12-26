#pragma once
#include <limits>
#include <variant>
#include <glm/glm.hpp>
#include "./enums/axis3D.h"
#include "./enums/gizmo_mode.h"
#include "./scene_entity_handle.h"

namespace vulkanDK {
   class surface_renderer;
}

namespace vulkanDK {
   struct raycast_hit_data {
      glm::vec2 bary_position  = glm::vec2{ 0, 0 };
      float     distance       = std::numeric_limits<float>::max();
      glm::vec3 position;
      glm::vec3 surface_normal = glm::vec3{ 0, 0, 0 };

      constexpr operator bool() const noexcept { return this->distance < std::numeric_limits<float>::max(); }
   };

   enum class raycast_hit_target {
      none,
      edit_gizmo,
      entity,
   };

   class raycast {
      public:
         struct test_flag {
            enum type {
               edit_gizmo = 0x00000001,
               landscapes = 0x00000002,
               meshes     = 0x00000004,
            };

            static constexpr const auto default_flags = landscapes | meshes;
         };
         using test_flags_t = std::underlying_type_t<test_flag::type>;

      public:
         raycast(surface_renderer& sr) : owner(sr) {}

         surface_renderer& owner;
         test_flags_t test_flags = test_flag::default_flags;
         glm::vec3 origin        = { 0.0, 0.0, 0.0 };
         glm::vec3 direction     = { 0.0, 0.0, 1.0 };

         struct {
            raycast_hit_target target = raycast_hit_target::none;
            std::variant<
               std::monostate,
               rendered_landscape_handle,
               rendered_mesh_handle
            > entity;
            struct {
               axis3D     axis = axis3D::x;
               gizmo_mode mode = gizmo_mode::none;
            } gizmo;
            //
            raycast_hit_data hit;
         } result;

         // Returns true if the passed-in hit is nearer to the raycast's primary stored hit, 
         // and so has *become* the raycast's primary stored hit.
         bool receive_hit(const raycast_hit_data&);

         void set_screen_relative_raycast(int viewport_x, int viewport_y);
   };
}