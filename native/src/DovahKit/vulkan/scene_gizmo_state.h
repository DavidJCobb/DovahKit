#pragma once
#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include "./enums/axis3D.h"
#include "./enums/gizmo_mode.h"

namespace vulkanDK {
   struct scene_gizmo_state {
      //
      // Vulkan expects precise member aligmnent; see: <https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/chap15.html#interfaces-resources-layout>
      // 

      struct flag {
         flag() = delete;
         enum type : uint32_t {
            mode_translate   = 0x00000001,
            mode_rotate      = 0x00000002,
            mode_scale       = 0x00000004,
            highlight_axis_x = 0x00000010,
            highlight_axis_y = 0x00000020,
            highlight_axis_z = 0x00000040,
         };

         static constexpr const std::underlying_type_t<type> all_default = 0;
         static constexpr const std::underlying_type_t<type> all_modes = mode_translate | mode_rotate | mode_scale;
      };
      using flags_t = std::underlying_type_t<flag::type>;

      alignas(16) glm::vec4 color_highlight = { 1, 1, 0, 1 }; // blended into an axis's base color if the axis is flagged as highlighted
      alignas(16) glm::vec3 color_x = { 1, 0, 0 };
      alignas( 4) flags_t   flags   = flag::all_default;
      alignas(16) glm::vec3 color_y = { 0, 1, 0 };
      // four bytes
      alignas(16) glm::vec3 color_z = { 0, 0, 1 };
      // four bytes
      alignas(16) glm::mat4 transform = glm::mat4(1);

      #pragma region Higher-level definitions
         #pragma region [gs]etters
            constexpr gizmo_mode get_mode() const noexcept;
            constexpr void set_mode(gizmo_mode) noexcept;

            constexpr bool is_axis_highlighted(axis3D) const noexcept;
            constexpr void set_axis_highlighted(axis3D, bool) noexcept;

            constexpr const glm::vec3& axis_color(axis3D) const noexcept;
            constexpr glm::vec3& axis_color(axis3D) noexcept;
         #pragma endregion
      #pragma endregion
   };
}

#include "./scene_gizmo_state.inl"