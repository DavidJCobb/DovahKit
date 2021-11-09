#include "swap_chain.h"
#include <cassert>
#include "config/frames_in_flight.h"
#include "logical_device.h"
#include "queue_family_info.h"
#include "render_pass.h"
#include "surface.h"
#include "surface_renderer.h"
#include "surface_support_info.h"

namespace {
   static constexpr auto desired_swap_chain_presentation_mode = VK_PRESENT_MODE_MAILBOX_KHR;
}

namespace vulkanDK {
   swap_chain::swap_chain(surface_renderer& c) : owner(c), depth_buffer(c) {
      this->frames_in_flight.resize(config::frames_in_flight_count);
   }
   swap_chain::~swap_chain() {
      this->teardown();
      this->frames_in_flight.clear();
   }

   void swap_chain::setup() {
      auto ssi = surface_support_info(this->owner);
      auto qfi = queue_family_info(this->owner);
      //
      VkSurfaceFormatKHR surfaceFormat;
      VkPresentModeKHR   presentMode;
      VkExtent2D         extent;
      uint32_t           imageCount;
      //
      #pragma region choose format
         assert(!ssi.formats.empty());
         surfaceFormat = ssi.formats[0]; // fallback
         for (const auto& current_format : ssi.formats) {
            if (current_format.format == VK_FORMAT_B8G8R8A8_SRGB && current_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
               surfaceFormat = current_format;
               break;
            }
         }
         this->format = surfaceFormat.format;
      #pragma endregion
      #pragma region choose presentation mode
         presentMode = VK_PRESENT_MODE_FIFO_KHR; // fallback
         for (const auto& current_mode : ssi.presentation_modes) {
            if (current_mode == desired_swap_chain_presentation_mode) {
               presentMode = current_mode;
               break;
            }
         }
      #pragma endregion
      #pragma region choose extent
         if (ssi.capabilities.currentExtent.width != UINT32_MAX) {
            extent = ssi.capabilities.currentExtent;
         } else {
            auto& min_e = ssi.capabilities.minImageExtent;
            auto& max_e = ssi.capabilities.maxImageExtent;
            //
            extent = this->owner.desired_surface_size();
            extent.width  = std::clamp(extent.width,  min_e.width,  max_e.width);
            extent.height = std::clamp(extent.height, min_e.height, max_e.height);
         }
         this->owner.surface_extent = extent;
      #pragma endregion
      #pragma region choose image count
         imageCount = ssi.capabilities.minImageCount + 1;
         if (ssi.capabilities.maxImageCount > 0 && imageCount > ssi.capabilities.maxImageCount) {
            imageCount = ssi.capabilities.maxImageCount;
         }
      #pragma endregion
      //
      auto create_info = VkSwapchainCreateInfoKHR{
         .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
         .surface          = this->owner.target.handle,
         .minImageCount    = imageCount,
         .imageFormat      = surfaceFormat.format,
         .imageColorSpace  = surfaceFormat.colorSpace,
         .imageExtent      = extent,
         .imageArrayLayers = 1,
         .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      };
      //
      auto list = qfi.families.list;
      if (qfi.families.graphics != qfi.families.presentation) {
         //
         // TODO: Apparently "exclusive" is faster for this case, but requires more complicated setup, 
         //       which the tutorial I'm following feels should be saved for later.
         // 
         // See: https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain#page_Creating-the-swap-chain
         //
         create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
         create_info.queueFamilyIndexCount = list.size();
         create_info.pQueueFamilyIndices   = list.data();
      } else {
         create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
         create_info.queueFamilyIndexCount = 0;       // clearing these two values is optional, but feels cleaner to me
         create_info.pQueueFamilyIndices   = nullptr; //
      }
      create_info.preTransform   = ssi.capabilities.currentTransform; // don't rotate or otherwise transform the image while rendering
      create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;   // disable alpha
      create_info.presentMode    = presentMode;
      create_info.clipped        = VK_TRUE;        // disable rendering of pixels covered (e.g. by other windows); good optimization, but prevents querying the colors of those pixels (e.g. for saving snapshots)
      create_info.oldSwapchain   = VK_NULL_HANDLE; // must be specified when rebuilding a swap chain; keep null for making a new swap chain
      //
      if (vkCreateSwapchainKHR(this->owner.device.handle, &create_info, nullptr, &this->handle) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::swap_chain::setup] Failed to create swap chain.");
      }
      //
      // Set up swap chain images:
      //
      this->_setup_depth_buffer();
      this->_setup_images();
      this->_setup_framebuffers();
      //
      // Set up frames in flight:
      //
      {
         auto& list = this->frames_in_flight;
         auto  size = list.size();
         for (size_t i = 0; i < size; ++i)
            list[i].setup(this->owner, i);
      }
      //
      // Set up materials:
      //
      this->_setup_materials();
   }
   void swap_chain::_setup_depth_buffer() {
      auto  extent = this->owner.surface_extent;
      auto  format = this->owner.find_depth_format();
      auto& db     = this->depth_buffer;
      db.create_image(extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      db.create_basic_view(format, VK_IMAGE_ASPECT_DEPTH_BIT);
      db.transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
   }
   void swap_chain::_setup_images() {
      auto     device = this->owner.device.handle;
      uint32_t image_count;
      //
      // Get the number of swapchain images:
      //
      vkGetSwapchainImagesKHR(device, this->handle, &image_count, nullptr);
      if (this->images.size() != image_count) {
         //
         // Get the image handles:
         //
         this->images.resize(image_count);
         std::vector<VkImage> image_handles(image_count);
         vkGetSwapchainImagesKHR(device, this->handle, &image_count, image_handles.data());
         for (size_t i = 0; i < image_count; ++i) {
            this->images[i].content.handle = image_handles[i];
         }
      }
      //
      // Set up the image views:
      //
      for (size_t i = 0; i < image_count; ++i) {
         this->images[i].create_basic_view(this->format, VK_IMAGE_ASPECT_COLOR_BIT);
      }
   }
   void swap_chain::_setup_framebuffers() {
      auto device = this->owner.device.handle;
      auto extent = this->owner.surface_extent;
      auto r_pass = this->owner.render_passes[0]->handle;
      //
      auto count = this->images.size();
      this->framebuffers.resize(count);
      for (size_t i = 0; i < count; i++) {
         //
         // Each swap chain image needs its own view for color attachment, but they can 
         // share a single view for depth attachment because our semaphores ensure that 
         // only one subpass is running at a time.
         //
         auto attachments = std::array{ this->images[i].view, this->depth_buffer.view };
         auto framebuffer_info = VkFramebufferCreateInfo{
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass      = r_pass,
            .attachmentCount = attachments.size(),
            .pAttachments    = attachments.data(),
            .width           = extent.width,
            .height          = extent.height,
            .layers          = 1,
         };
         if (vkCreateFramebuffer(device, &framebuffer_info, nullptr, &this->framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("[vulkanDK::swap_chain::_setup_framebuffers] Failed to create a framebuffer.");
         }
      }
   }
   void swap_chain::_setup_materials() {
      auto count = this->material_definitions.size();
      if (count == 0) {
         qDebug("[vulkanDK::swap_chain::_setup_materials] WARNING: no material definitions provided before swap chain setup; is this intentional?");
      }
      this->materials.clear();
      this->materials.resize(count);
      for (size_t i = 0; i < count; ++i) {
         this->materials[i] = material(this->owner, this->material_definitions[i]);
      }
   }

   void swap_chain::teardown() {
      this->materials.clear();
      //
      for (auto& fb : this->framebuffers) {
         vkDestroyFramebuffer(this->owner.device.handle, fb, nullptr);
         fb = VK_NULL_HANDLE;
      }
      for (auto& image : this->images) {
         image.destroy_view();
      }
      this->depth_buffer.teardown();
      vkDestroySwapchainKHR(this->owner.device.handle, this->handle, nullptr);
      this->handle = VK_NULL_HANDLE;
   }

   swap_chain::pending_frame swap_chain::advance_frame() {
      constexpr auto no_timeout = UINT64_MAX;
      //
      auto& frame = this->frames_in_flight[this->current_frame];
      this->current_frame = (this->current_frame + 1) % this->frames_in_flight.size();
      vkWaitForFences(this->owner.device.handle, 1, &frame.fence, VK_TRUE, no_timeout);
      //
      auto result = pending_frame{ frame };
      result.result = vkAcquireNextImageKHR(this->owner.device.handle, this->handle, no_timeout, frame.semaphores.image_available, VK_NULL_HANDLE, &result.sc_image_index);
      return result;
   }
   void swap_chain::confirm_frame(pending_frame& pf) {
      {
         auto& handle = this->images_in_flight[pf.sc_image_index];
         //
         // Check if a previous frame is using this image.
         //
         if (handle != VK_NULL_HANDLE) {
            vkWaitForFences(this->owner.device.handle, 1, &handle, VK_TRUE, UINT64_MAX);
         }
         //
         // Mark the image as now being in use by this frame.
         //
         handle = pf.frame.fence;
      }
      pf.frame.draw(this->framebuffers[pf.sc_image_index]);
      //
      auto signal_semaphores  = std::array{ pf.frame.semaphores.render_finished };
      auto swap_chain_handles = std::array{ this->handle };
      auto presentation_info  = VkPresentInfoKHR{
         .sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
         .waitSemaphoreCount  = signal_semaphores.size(),
         .pWaitSemaphores     = signal_semaphores.data(),
         .swapchainCount      = swap_chain_handles.size(),
         .pSwapchains         = swap_chain_handles.data(),
         .pImageIndices       = &pf.sc_image_index,
         .pResults            = nullptr,
      };
      pf.result = vkQueuePresentKHR(this->owner.device.queues.presentation, &presentation_info);
   }
}