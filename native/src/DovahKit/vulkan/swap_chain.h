#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "image.h"
#include "material.h"

namespace vulkanDK {
   class context;
   class frame_in_flight;

   class swap_chain : no_copy {
      public:
         swap_chain(context&);
         ~swap_chain();

         swap_chain(swap_chain&&) noexcept;

         context& owner;
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

         void setup();
         void teardown();

      protected:
         void _setup_depth_buffer();
         void _setup_images();
         void _setup_framebuffers(); // depends on depth buffer and images
         void _setup_materials();
   };
}
