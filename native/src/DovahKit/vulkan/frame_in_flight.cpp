#include "frame_in_flight.h"
#include <cassert>
#include "exceptions.h"
#include "surface_renderer.h"

namespace vulkanDK {
   frame_in_flight::~frame_in_flight() {
      if (!this->owner) {
         assert(this->fence == VK_NULL_HANDLE);
         return;
      }
      //
      // Other resources have their own destructors, but we need to release these explicitly:
      //
      auto* ld = this->owner->logical_device;
      if (this->fence != VK_NULL_HANDLE) {
         vkDestroySemaphore(ld, this->semaphores.render_finished, nullptr);
         vkDestroySemaphore(ld, this->semaphores.image_available, nullptr);
         vkDestroyFence    (ld, this->fence, nullptr);
      }
   }

   void frame_in_flight::setup(surface_renderer& sr) {
      this->owner = &sr;
      //
      this->_setup_semaphores();
   }
   //
   void frame_in_flight::_setup_semaphores() {
      assert(this->owner);
      auto device = this->owner->logical_device;
      //
      auto semaphore_info = VkSemaphoreCreateInfo{
         .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      };
      auto fence_info = VkFenceCreateInfo{
         .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
         //
         // Our drawFrame code waits until a frame is signalled, but frames start off unsignalled by 
         // default. This means that it'll wait forever, unless we initialize the frame as signalled.
         //
         .flags = VK_FENCE_CREATE_SIGNALED_BIT,
      };
      if (auto result = vkCreateSemaphore(device, &semaphore_info, nullptr, &this->semaphores.image_available); result != VK_SUCCESS) {
         throw result_exception(result, "[frame_in_flight::_setup_semaphores] Failed to create frame-in-flight semaphore (image-available).");
      }
      if (auto result = vkCreateSemaphore(device, &semaphore_info, nullptr, &this->semaphores.render_finished); result != VK_SUCCESS) {
         throw result_exception(result, "[frame_in_flight::_setup_semaphores] Failed to create frame-in-flight semaphore (render-finished).");
      }
      if (auto result = vkCreateFence(device, &fence_info, nullptr, &this->fence); result != VK_SUCCESS) {
         throw result_exception(result, "[frame_in_flight::_setup_semaphores] Failed to create frame-in-flight fence.");
      }
   }
}