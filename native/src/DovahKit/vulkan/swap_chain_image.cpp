#include "swap_chain_image.h"
#include <stdexcept>
#include "command_buffer.h"
#include "exceptions.h"
#include "frame_in_flight.h"
#include "render_pass.h"
#include "rendered_light.h"
#include "rendered_mesh.h"
#include "scene_global_state.h"
#include "surface_renderer.h"
#include "config/scene_limits.h"
#include "config/shadow_maps.h"
#include "config/use_inverted_depth.h"

namespace {
   static constexpr bool debug_log_scene_object_lifetimes = false;
}

namespace vulkanDK {
   swap_chain_image::swap_chain_image(surface_renderer& o, size_t i) {
      this->owner = &o;
      this->my_index = i;
   }
   swap_chain_image::~swap_chain_image() {
      this->teardown();
   }

   swap_chain_image::swap_chain_image(swap_chain_image&& o) noexcept {
      *this = std::move(o);
   }
   swap_chain_image& swap_chain_image::operator=(swap_chain_image&& o) noexcept {
      std::swap(this->image,              o.image);
      std::swap(this->final_blit_command, o.final_blit_command);
      return *this;
   }

   void swap_chain_image::setup(surface_renderer& o, size_t i) {
      if (this->owner) {
         assert(this->owner == &o);
      } else {
         this->owner = &o;
      }
      this->my_index = i;
      this->setup();
   }
   void swap_chain_image::setup() {
      this->final_blit_command = command_buffer(*this->owner);
      this->owner->set_debug_object_name(this->final_blit_command.handle, QString("Swap Chain Image %1: Command Buffer: Final Blit").arg(this->my_index).toStdString());
   }
   void swap_chain_image::record_final_blit_command() {
      auto& command_buffer = this->final_blit_command;
      auto  command_handle = command_buffer.handle;
      //
      command_buffer.reset(0);
      if (auto result = command_buffer.top_level_begin(0); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::swap_chain_image::_record_final_blit_command] Failed to begin the command buffer.");
      }
      //
      static constexpr auto color_image_layout_for_blit = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      static constexpr auto color_image_access_for_blit = VK_ACCESS_TRANSFER_READ_BIT;
      //
      {  // Blit to swap chain image
         const auto& extent      = this->owner->surface_extent;
         const auto  blit_region = VkImageBlit{
            .srcSubresource = {
               .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
               .mipLevel       = 0,
               .baseArrayLayer = 0,
               .layerCount     = 1,
            },
            .srcOffsets = {
               { .x = 0, .y = 0, .z = 0 },
               {
                  .x = (int32_t)extent.width,
                  .y = (int32_t)extent.height,
                  .z = 1,
               },
            },
            .dstSubresource = {
               .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
               .mipLevel       = 0,
               .baseArrayLayer = 0,
               .layerCount     = 1,
            },
            .dstOffsets = {
               {.x = 0, .y = 0, .z = 0 },
               {
                  .x = (int32_t)extent.width,
                  .y = (int32_t)extent.height,
                  .z = 1,
               },
            },
         };

         this->owner->canvas.color.transition_layout(
            command_buffer,
            VK_IMAGE_ASPECT_COLOR_BIT,
            surface_renderer::color_target_layout_for_render,
            color_image_layout_for_blit,
            surface_renderer::color_target_access_for_render,
            color_image_access_for_blit
         );
         this->image.transition_layout(command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);
         vkCmdBlitImage(command_handle,
            this->owner->canvas.color.handle,
            this->owner->canvas.color.current.layout,
            this->image.handle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit_region,
            VK_FILTER_NEAREST
         );
         this->image.transition_layout(command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 0);
      }
      this->owner->canvas.color.transition_layout(
         command_buffer,
         VK_IMAGE_ASPECT_COLOR_BIT,
         color_image_layout_for_blit,
         surface_renderer::color_target_layout_for_render,
         color_image_access_for_blit,
         surface_renderer::color_target_access_for_render
      );
      if (auto result = command_buffer.finish(); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::swap_chain_image::_record_final_blit_command] Failed to finish the command buffer.");
      }
   }

   void swap_chain_image::handle_resize() {
      this->record_final_blit_command();
   }

   void swap_chain_image::teardown() {
      if (!this->owner)
         return;
      this->image.teardown();
   }

   void swap_chain_image::draw(frame_in_flight& fif) {
      this->_hook_to_frame(fif);
      //
      fif.prepare_for_render();
      fif.prepare_indirect_draws();
      fif.record_compute_cull_commands();
      fif.record_graphics_commands();
      //
      // Submit our command buffers.
      // 
      fif.submit_compute_cull_commands();
      // 
      // Our submit operation here will wait for all of the "wait semaphores" we provide 
      // to be signalled before beginning. When all command buffers listed in the submit 
      // operation have completed execution, it will signal the "signal semaphores" that 
      // we've provided.
      // 
      // In this case,  we're waiting on our "image available" semaphore,  which will be 
      // signalled when the swap chain image  provided by vkAcquireNextImageKHR is ready 
      // for use; and we're signalling our "render finished" semaphore. The rendere will 
      // use our "render finished" semaphore as  the "wait semaphore" for presenting the 
      // rendered image via vkQueuePresentKHR.
      // 
      // We also have  a fence that we use to  indicate when our command  buffers are in 
      // the middle of being executed, so we'll  reset that fence just before we submit, 
      // and pass the fence as an argument for the queue-submit call. The call will then 
      // signal our fence when the command buffers  have finished running. We need to do 
      // all this so that we can safely reuse our command buffer when rendering.
      //
      fif.submit_graphics_commands({
         this->final_blit_command.handle,
      });
   }
   
   void swap_chain_image::_hook_to_frame(frame_in_flight& fif) {
      auto& handles = this->current_fence_handles;
      //
      // It may be the case that the last frame  to use this swap chain image is still 
      // drawing to it.  We can wait on that  frame's fence in order  to know when its 
      // command buffers have finished executing.
      //
      if (!handles.empty()) {
         //
         // This branch would only *not* run when using a swap chain image for the first 
         // time, when it's never been bound to a frame-in-flight before.
         //
         if (auto result = handles.wait_on_all(this->owner->logical_device); result != VK_SUCCESS) {
            throw result_exception(result, "[swap_chain_image::_hook_to_frame] Wait on previously-bound frame-in-flight failed.");
         }
      }
      //
      // Mark our swap chain  image as being in use for the new  frame and its command 
      // buffers.
      //
      handles = fif.fences;
   }
}