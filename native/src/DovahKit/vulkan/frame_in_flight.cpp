#include "frame_in_flight.h"
#include <cassert>
#include <stdexcept>
#include "config/scene_limits.h"
#include "logical_device.h"
#include "render_pass.h"
#include "rendered_mesh.h"
#include "scene_global_state.h"
#include "surface_renderer.h"

#include "scene.h"

namespace vulkanDK {
   #pragma region frame_render_pass
   #pragma endregion

   #pragma region frame_in_flight
   frame_in_flight::~frame_in_flight() {
      if (!this->owner) {
         assert(this->fence == VK_NULL_HANDLE);
         return;
      }
      //
      // Other resources have their own destructors, but we need to release these explicitly:
      //
      auto* ld = this->owner->device.handle;
      if (this->fence != VK_NULL_HANDLE) {
         vkDestroySemaphore(ld, this->semaphores.render_finished, nullptr);
         vkDestroySemaphore(ld, this->semaphores.image_available, nullptr);
         vkDestroyFence    (ld, this->fence, nullptr);
      }
   }

   void frame_in_flight::setup(surface_renderer& sr, size_t which_am_i) {
      this->owner    = &sr;
      this->my_index = which_am_i;
      //
      this->_setup_semaphores();
      this->_setup_shader_parameter_buffers();
      this->_setup_descriptor_sets(); // no corresponding teardown step needed; the descriptor pool will yeet these when it gets torn down
      this->_setup_command_buffers();
   }
   //
   void frame_in_flight::_setup_semaphores() {
      assert(this->owner);
      auto device = this->owner->device.handle;
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
         this->shader_params.uniform = this->owner->device.create_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      }
      {  // Object data list
         constexpr VkDeviceSize rosp_buffer_size = config::max_rendered_meshes * sizeof(rendered_mesh::shader_parameters);
         this->shader_params.object_data = this->owner->device.create_buffer(rosp_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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
      if (vkAllocateDescriptorSets(this->owner->device.handle, &alloc_info, this->descriptor_sets.data()) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanKD::frame_in_flight::_setup_descriptor_sets] Failed to allocate descriptor sets.");
      }
   }
   void frame_in_flight::_setup_command_buffers() {
      for (auto& pass : this->render_passes) {
         pass.command_buffers.resize(1);
         //
         pass.command_buffers = command_buffer::create_in_bulk(*this->owner, pass.command_buffers.size());
         for(auto& cb : pass.command_buffers)
            if (cb.handle == VK_NULL_HANDLE)
               throw std::runtime_error("[vulkanDK::frame_in_flight::_setup_command_buffers] Failed to allocate command buffers.");
         //
         pass.command_buffers_invalid = true;
      }
   }

   void frame_in_flight::draw(VkFramebuffer target_framebuffer) {
      this->_update_shader_object_data_buffer();
      this->_update_shader_texture_descriptors(); // can invalidate command buffers, so must run before we check whether command buffers need refilling
      for (auto& rp : this->render_passes) {
         if (!rp.command_buffers_invalid)
            continue;
         this->_refill_command_buffers(target_framebuffer);
      }
      std::vector<VkCommandBuffer> command_buffer_handles;
      {
         for (auto& rp : this->render_passes)
            for (auto& cb : rp.command_buffers)
               command_buffer_handles.push_back(cb.handle);
      }
      auto wait_semaphores   = std::array{ this->semaphores.image_available };
      auto signal_semaphores = std::array{ this->semaphores.render_finished };
      VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
      auto submit_info = VkSubmitInfo{
         .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
         .waitSemaphoreCount   = wait_semaphores.size(),
         .pWaitSemaphores      = wait_semaphores.data(),
         .pWaitDstStageMask    = waitStages,
         .commandBufferCount   = (uint32_t)command_buffer_handles.size(),
         .pCommandBuffers      = command_buffer_handles.data(),
         .signalSemaphoreCount = signal_semaphores.size(),
         .pSignalSemaphores    = signal_semaphores.data(),
      };
      vkResetFences(this->owner->device.handle, 1, &this->fence);
      if (vkQueueSubmit(this->owner->device.queues.graphics, 1, &submit_info, this->fence) != VK_SUCCESS) {
         throw std::runtime_error("failed to submit draw command buffer!");
      }
   }
   //
   scene& frame_in_flight::get_scene() {
      assert(this->owner);
      return this->owner->scene;
   }
   void frame_in_flight::_update_shader_object_data_buffer() {
      using entry_type = rendered_mesh::shader_parameters;
      constexpr auto entry_size = sizeof(entry_type);

      constexpr bool map_only_what_is_necessary = true;

      auto& scene  = this->get_scene();
      auto& buffer = this->shader_params.object_data;
      //
      auto& ro     = scene.meshes;
      auto  count  = ro.size();
      assert(count < config::max_rendered_meshes);
      VkDeviceSize size = count * entry_size;
      //
      uint32_t flag = 1 << this->my_index;
      //
      size_t first_dirty = 0;
      size_t last_dirty  = 0;
      bool   any_dirty   = false;
      if constexpr (map_only_what_is_necessary) {
         for (size_t i = 0; i < count; ++i) {
            const auto& item = ro[i];
            if (item.pending_delete || item.empty()) // TODO: could skip the "empty" check if we force the dirty-flags to 0 on empty items and set to -1 when filling them again
               continue;
            if (item.frame_dirty_flags & flag) {
               if (!any_dirty) {
                  first_dirty = i;
                  any_dirty   = true;
               }
               last_dirty = i;
            }
         }
      } else {
         for (size_t i = 0; i < count; ++i) {
            const auto& item = ro[i];
            if (item.pending_delete || item.empty())
               continue;
            if (item.frame_dirty_flags & flag) {
               first_dirty = i;
               any_dirty   = true;
               break;
            }
         }
      }
      //
      if (any_dirty) {
         constexpr bool map_only_what_is_necessary = true;
         //
         entry_type* data = nullptr;
         if constexpr (map_only_what_is_necessary) {
            VkDeviceSize offset = first_dirty * entry_size;
            VkDeviceSize length = (last_dirty - first_dirty + 1) * entry_size;
            data = (entry_type*)buffer.map_memory(offset, length);
            for (size_t i = first_dirty; i <= last_dirty; ++i) {
               auto& item = ro[i];
               if (item.pending_delete || item.empty())
                  continue;
               if (!(item.frame_dirty_flags & flag))
                  continue;
               auto& src = ro[i].shader_params;
               auto& dst = data[i - first_dirty];
               memcpy(&dst, &src, entry_size);
               //
               ro[i].frame_dirty_flags &= ~flag;
            }
         } else {
            data = (entry_type*)buffer.map_memory();
            for (size_t i = first_dirty; i < count; ++i) {
               auto& item = ro[i];
               if (item.pending_delete || item.empty())
                  continue;
               if (!(item.frame_dirty_flags & flag))
                  continue;
               auto& src = ro[i].shader_params;
               auto& dst = data[i];
               memcpy(&dst, &src, entry_size);
               //
               item.frame_dirty_flags &= ~flag;
            }
         }
         buffer.unmap_memory(data);
      }
   }
   void frame_in_flight::_update_shader_texture_descriptors() {
      auto&    scene = this->get_scene();
      auto&    list  = scene.textures;
      uint32_t size  = list.size();
      //
      bool  needs_null_texture = this->owner->device.physical.support.descriptor_bindings.null_handles;
      auto& null_texture       = this->owner->null_texture;
      //
      struct pending_write {
         uint32_t start = 0;
         std::vector<VkDescriptorImageInfo> entries;
         //
         inline uint32_t end() const noexcept { return this->start + this->entries.size(); }
      };
      //
      std::vector<pending_write> writes;
      auto flag      = frames_in_flight_mask::mask_type(1) << this->my_index;
      bool deletions = false;
      for (uint32_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (!(item.frame_dirty_flags & flag))
            continue;
         if (item.content.handle == VK_NULL_HANDLE) // deleted texture
            continue;
         item.frame_dirty_flags &= ~flag;
         auto view = item.content.view;
         if (item.pending_delete) {
            if (!needs_null_texture) {
               view = null_texture.view;
               assert(view != VK_NULL_HANDLE && "Null descriptor handles aren't supported, but we never set up our null texture!");
            } else {
               view = VK_NULL_HANDLE;
            }
            deletions = true; // TODO: optimize by only setting this if the texture has no dirty flags (that correspond to actual frames) remaining
         }
         //
         if (!writes.empty()) { // group consecutive textures into a single write, when possible
            auto& back = writes.back();
            if (back.end() == i) {
               back.entries.emplace_back(VkDescriptorImageInfo{
                  .sampler     = nullptr,
                  .imageView   = view,
                  .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
               });
               continue;
            }
         }
         writes.emplace_back(pending_write{
            .start   = i,
            .entries = {
               {
                  .sampler     = nullptr,
                  .imageView   = view,
                  .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
               }
            },
         });
      }
      if (writes.empty())
         return;
      //
      auto& target_set = this->descriptor_sets[0];
      //
      std::vector<VkWriteDescriptorSet> write_info(writes.size());
      for (size_t i = 0; i < writes.size(); ++i) {
         auto& src  = writes[i];
         auto& info = writes[i].entries;
         //
         write_info[i] = VkWriteDescriptorSet{ // texture array
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = target_set,
            .dstBinding      = 3, // this should match the binding value in the shader
            .dstArrayElement = src.start,
            .descriptorCount = (uint32_t)info.size(),
            .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo      = info.data(),
         };
      }
      vkUpdateDescriptorSets(this->owner->device.handle, (uint32_t)write_info.size(), write_info.data(), 0, nullptr);
      //
      // Updating a descriptor set will invalidate any command buffers using it; they must 
      // be reset and their queue regenerated:
      //
      this->invalidate_all_command_buffers();
   }
   void frame_in_flight::_refill_command_buffers(VkFramebuffer framebuffer) {
      auto& pass_state     = this->render_passes[0];
      auto  pass_handle    = this->owner->render_passes[0]->handle;
      auto& scene          = this->owner->scene;
      auto  command_buffer = pass_state.command_buffers[0].handle;
      pass_state.command_buffers_invalid = false;
      //
      vkResetCommandBuffer(command_buffer, 0);
      auto buffer_begin_info = VkCommandBufferBeginInfo{
         .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
         .flags            = 0,
         .pInheritanceInfo = nullptr,
      };
      if (vkBeginCommandBuffer(command_buffer, &buffer_begin_info) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::frame_in_flight::_refill_command_buffers] Failed to begin recording command buffer.");
      }
      //
      auto clear_values = std::array{
         //
         // Values here should match the attachments we're using.
         //
         VkClearValue{ .color        = {0, 0, 0, 1} }, // color attachment uses VK_ATTACHMENT_LOAD_OP_CLEAR; this is the value to clear with
         VkClearValue{ .depthStencil = {1.0, 0} },     // depth attachment uses VK_ATTACHMENT_LOAD_OP_CLEAR; this is the depth range to celar with
      };
      auto pass_begin_info = VkRenderPassBeginInfo{
         .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
         .renderPass  = pass_handle,
         .framebuffer = framebuffer,
         .renderArea  = {
            .offset = { 0, 0 },
            .extent = this->owner->surface_extent,
         },
         .clearValueCount = (uint32_t)clear_values.size(),
         .pClearValues    = clear_values.data(),
      };

      bool any_deleted_objects = false;

      vkCmdBeginRenderPass(command_buffer, &pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
      {
         //
         // We'd want to pre-sort objects by material, and re-bind descriptor sets and pipelines 
         // with each new material.
         //
         const auto& material = this->owner->swap_chain.materials[0]; // TODO: find a better way to retrieve this
         vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material.pipeline.layout, 0, this->descriptor_sets.size(), this->descriptor_sets.data(), 0, nullptr);
         vkCmdBindPipeline      (command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material.pipeline.handle);
         //
         {
            VkDeviceSize offset = 0;
            for (size_t j = 0; j < scene.meshes.size(); ++j) {
               auto& ro  = scene.meshes[j];
               auto& vib = ro.vertex_and_index_buffer;
               //
               if (ro.empty() || ro.pending_delete)
                  continue;

               auto pc = rendered_mesh::push_constant{
                  .object_index  = (int32_t)j,
                  .texture_index = (int32_t)ro.texture_index,
               };
               vkCmdPushConstants(
                  command_buffer,
                  material.pipeline.layout,
                  VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
                  0,
                  sizeof(pc),
                  (void*)&pc
               );
               ro.draw_call(command_buffer);
            }
            qDebug("[vulkanDK::frame_in_flight::_refill_command_buffers] Command buffer: processed %u objects.", scene.meshes.size());
         }
      }
      vkCmdEndRenderPass(command_buffer);
      if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::frame_in_flight::_refill_command_buffers] Failed to record a command buffer.");
      }
   }

   void frame_in_flight::invalidate_all_command_buffers() {
      for (auto& rp : this->render_passes)
         rp.command_buffers_invalid = true;
   }
   #pragma endregion
}