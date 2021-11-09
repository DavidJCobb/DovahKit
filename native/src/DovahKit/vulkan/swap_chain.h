#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "frame_in_flight.h"
#include "image.h"
#include "material.h"

namespace vulkanDK {
   class surface_renderer;
   class frame_in_flight;

   class swap_chain : no_copy {
      public:
         swap_chain(surface_renderer&);
         ~swap_chain();

         swap_chain(swap_chain&&) noexcept;

         surface_renderer& owner;
         std::vector<material_definition*> material_definitions; // owns
         //
         VkSwapchainKHR handle = VK_NULL_HANDLE;
         VkFormat       format = VK_FORMAT_UNDEFINED;
         concrete_image depth_buffer; // only one should be needed: we only use it during rendering, not presentation, and we render one frame at a time synched via subpass dependencies
         //
         std::vector<material>        materials;
         std::vector<image_and_view>  images;
         std::vector<VkFramebuffer>   framebuffers;
         std::vector<frame_in_flight> frames_in_flight;
         //
         std::vector<VkFence> images_in_flight; // handles. if images[i] is in flight, then images_in_flight[i] == frames_in_flight[x].fence; else, it's a null handle
         size_t current_frame = 0;

         void setup();
         void teardown();

         struct pending_frame {
            frame_in_flight& frame;
            uint32_t         sc_image_index;
            VkResult         result;
         };
         pending_frame advance_frame();
         void confirm_frame(pending_frame&);

      protected:
         void _setup_depth_buffer();
         void _setup_images();
         void _setup_framebuffers(); // depends on depth buffer and images
         void _setup_materials();
   };
}
