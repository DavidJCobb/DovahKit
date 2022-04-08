#pragma once
#include <array>
#include <type_traits>
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "buffer.h"
#include "command_buffer.h"
#include "frame_in_flight.h"
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
         //
         bool recorded_final_blit_command = false;

      public:
         swap_chain_image() {}
         swap_chain_image(surface_renderer&, size_t my_index);
         ~swap_chain_image();

         swap_chain_image(swap_chain_image&&) noexcept;
         swap_chain_image& operator=(swap_chain_image&&) noexcept;

         frame_in_flight_fence_set current_fence_handles;
         //
         image_and_view image;
         command_buffer final_blit_command;

         void setup(surface_renderer&, size_t my_index); // calls _record_final_blit_command
         void setup();
         void record_final_blit_command(); // requires surface renderer extent and swap chain image handle

         void handle_resize(); // calls record_final_blit_command

         void teardown();
         void draw(frame_in_flight&);

      protected:
         // draw steps:
         void _hook_to_frame(frame_in_flight&);
   };
}