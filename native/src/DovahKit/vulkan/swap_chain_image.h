#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "buffer.h"
#include "command_buffer.h"
#include "image.h"
#include "surface_renderer_descriptor_group.h"
#include "overlays/fps.h"
#include "overlays/world_axes.h"

namespace vulkanDK {
   class frame_in_flight;
   class scene;
   class surface_renderer;

   class swap_chain_image : no_copy {
      protected:
         surface_renderer* owner = nullptr;
         size_t my_index = -1;
      public:
         swap_chain_image() {}
         swap_chain_image(surface_renderer&, size_t my_index);
         ~swap_chain_image();

         swap_chain_image(swap_chain_image&&) noexcept;
         swap_chain_image& operator=(swap_chain_image&&) noexcept;

         surface_renderer_image_view image;
         VkFramebuffer framebuffer = VK_NULL_HANDLE;
         //
         descriptor_set_group descriptor_sets;
         std::vector<command_buffer> command_buffers;
         struct {
            buffer uniform;     // per-scene  data which can be updated without having to re-record command buffers
            buffer object_data; // per-object data which can be updated without having to re-record command buffers
            buffer light_data;  // per-light  data which can be updated without having to re-record command buffers
         } shader_params;
         struct {
            overlays::fps        fps;
            overlays::world_axes world_axes;
         } overlays;
         //
         VkFence current_fence_handle = VK_NULL_HANDLE;
         bool command_buffers_invalid = true;

         void setup(surface_renderer&, size_t my_index);
         void setup();
         void setup_descriptor_sets();

         void teardown_descriptor_sets(); // only need to tear these down if we reuse the descriptor pool; not needed if we also destroy the descriptor pool
         void teardown();

         void draw(frame_in_flight&);

         void invalidate_all_command_buffers();

      protected:
         void _setup_shader_parameter_buffers();
         void _setup_descriptor_sets();
         void _setup_command_buffers();

         // draw steps:
         void _hook_to_frame(frame_in_flight&);
         //
         scene& get_scene();
         void _update_shader_global_scene_state();
         void _update_shader_lights_data_buffer();
         void _update_shader_object_data_buffer();
         void _update_shader_texture_descriptors();
         void _refill_command_buffers();
         void _refill_fps_overlay_command_buffer();
   };
}