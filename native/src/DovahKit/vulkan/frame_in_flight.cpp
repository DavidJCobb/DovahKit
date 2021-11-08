#include "frame_in_flight.h"
#include <cassert>
#include <stdexcept>
#include "config/scene_limits.h"
#include "context.h"
#include "rendered_mesh.h"
#include "scene_global_state.h"

namespace vulkanDK {
   #pragma region frame_render_pass
   #pragma endregion

   #pragma region frame_in_flight
   frame_in_flight::frame_in_flight(context& c) : owner(&c) {
      this->_setup_semaphores();
      this->_setup_shader_parameter_buffers();
      this->_setup_descriptor_sets();
      this->_setup_command_buffers();
      static_assert(false);
   }
   frame_in_flight::~frame_in_flight();

   void frame_in_flight::_setup_semaphores() {
      assert(this->owner);
      auto device = this->owner->logical_device();
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
      if (vkCreateSemaphore(device, &semaphore_info, nullptr, &this->semaphores.image_available) != VK_SUCCESS) {
         throw std::runtime_error("[frame_in_flight::_setup_semaphores] Failed to create frame-in-flight semaphore (image-available).");
      }
      if (vkCreateSemaphore(device, &semaphore_info, nullptr, &this->semaphores.render_finished) != VK_SUCCESS) {
         throw std::runtime_error("[frame_in_flight::_setup_semaphores] Failed to create frame-in-flight semaphore (render-finished).");
      }
      if (vkCreateFence(device, &fence_info, nullptr, &this->fence) != VK_SUCCESS) {
         throw std::runtime_error("[frame_in_flight::_setup_semaphores] Failed to create frame-in-flight fence.");
      }
   }
   void frame_in_flight::_setup_shader_parameter_buffers() {
      {  // Scene global state, as a uniform buffer object
         constexpr VkDeviceSize buffer_size = sizeof(scene_global_state);
         this->shader_params.uniform = this->owner->owner.create_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      }
      {  // Object data list
         constexpr VkDeviceSize rosp_buffer_size = config::max_rendered_meshes * sizeof(rendered_mesh::shader_parameters);
         this->shader_params.object_data = this->owner->owner.create_buffer(rosp_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      }
   }
   void frame_in_flight::_setup_descriptor_sets() {
      const auto& layout = this->owner->descriptor_set_definition;
      //
      // Each descriptor set can have a single descriptor binding that acts as a variable-length 
      // array of descriptors. However, we have to provide suitable maximums for these lists via 
      // an extension struct.
      //
      std::array<VkDescriptorSetLayout, 1> layouts = { layout.handle };
      std::array<uint32_t, 1> variable_counts; // one count per set; sets with no variable-length array will ignore their respective count
      {
         auto& bl = layout.bindings; // TODO: if we have multiple sets per frame, pick the right set layout
         for (size_t i = 0; i < layouts.size(); ++i) {
            auto& vc = variable_counts[i];
            for (size_t j = 0; j < bl.size(); ++j) {
               auto& binding = bl[j];
               if (binding.flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
                  assert(vc == 0            && "A descriptor set is not allowed to have multiple variable-length descriptor bindings.");
                  assert(j == bl.size() - 1 && "If a descriptor set has a variable-length descriptor binding, it must be the last binding in the list.");
                  vc = binding.count;
               }
            }
         }
      }
      auto variable_count_info = VkDescriptorSetVariableDescriptorCountAllocateInfo{
         .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
         .descriptorSetCount = (uint32_t)variable_counts.size(),
         .pDescriptorCounts  = variable_counts.data(),
      };
      auto alloc_info = VkDescriptorSetAllocateInfo{
         .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
         .pNext              = &variable_count_info,
         .descriptorPool     = this->owner->descriptor_pool,
         .descriptorSetCount = (uint32_t)layouts.size(),
         .pSetLayouts        = layouts.data(),
      };
      //
      this->descriptor_sets.resize(layouts.size());
      //
      // WARNING: If the descriptor pool has an inadequate size, vkAllocateDescriptorSets 
      // MAY fail with an VK_ERROR_POOL_OUT_OF_MEMORY error code... However, some device 
      // drivers may try to solve the problem internally instead, which means that that 
      // particular class of error will not fail consistently across all hardware. Beware. 
      //
      if (vkAllocateDescriptorSets(this->owner->logical_device(), &alloc_info, this->descriptor_sets.data()) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanKD::frame_in_flight::_setup_descriptor_sets] Failed to allocate descriptor sets.");
      }
   }
   void frame_in_flight::_setup_command_buffers() {
      for (auto& pass : this->render_passes) {
         pass.command_buffers.resize(1);
         //
         std::vector<VkCommandBuffer> handles;
         handles.resize(pass.command_buffers.size());
         //
         auto alloc_info = VkCommandBufferAllocateInfo{
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = this->owner->command_pool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = (uint32_t)handles.size(),
         };
         if (vkAllocateCommandBuffers(this->owner->logical_device(), &alloc_info, handles.data()) != VK_SUCCESS) {
            throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to allocate command buffers.");
         }
         //
         for (size_t i = 0; i < handles.size(); ++i) {
            //
            // TODO: Make it so you can't set the command buffer's handle directly, but rather must go 
            // through an accessor that also requires a context pointer.
            //
            pass.command_buffers[i].owner  = this->owner;
            pass.command_buffers[i].handle = handles[i];
         }
         pass.command_buffers_invalid = true;
      }
   }


   void frame_in_flight::draw(VkFramebuffer target_framebuffer);

   void frame_in_flight::invalidate_all_command_buffers();

   void frame_in_flight::teardown_for_resize();
   #pragma endregion
}