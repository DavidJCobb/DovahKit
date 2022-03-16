#include "swap_chain_image.h"
#include <stdexcept>
#include "command_buffer.h"
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
      //
      this->overlays.world_axes.set_owner(o);
   }
   swap_chain_image::~swap_chain_image() {
      this->teardown();
   }

   swap_chain_image::swap_chain_image(swap_chain_image&& o) noexcept {
      *this = std::move(o);
   }
   swap_chain_image& swap_chain_image::operator=(swap_chain_image&& o) noexcept {
      std::swap(this->image,           o.image);
      std::swap(this->descriptor_sets, o.descriptor_sets);
      std::swap(this->command_buffers, o.command_buffers);
      std::swap(this->shader_params,   o.shader_params);
      //
      std::swap(this->current_fence_handle,    o.current_fence_handle);
      std::swap(this->command_buffers_invalid, o.command_buffers_invalid);
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
      this->_setup_shader_parameter_buffers();
      this->_setup_command_buffers();
      //
      // Overlays:
      //
      this->overlays.fps.setup_shader_parameter_buffers(*this->owner);
      this->overlays.fps.create_geometry(*this->owner);
      //
      this->overlays.world_axes.set_owner(*this->owner);
      this->overlays.world_axes.setup_shader_parameter_buffers();
      this->overlays.world_axes.create_geometry();
   }
   void swap_chain_image::_setup_shader_parameter_buffers() {
      {
         constexpr VkDeviceSize buffer_size = sizeof(scene_global_state);
         this->shader_params.uniform = this->owner->create_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      }
      {
         constexpr VkDeviceSize rosp_buffer_size = config::max_rendered_meshes * sizeof(rendered_mesh::shader_parameters);
         this->shader_params.object_data = this->owner->create_buffer(rosp_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      }
      {
         constexpr VkDeviceSize rlsp_buffer_size = config::max_lights_in_scene * sizeof(rendered_light::shader_parameters);
         this->shader_params.light_data = this->owner->create_buffer(rlsp_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
         //
         auto* data = this->shader_params.light_data.map_memory();
         memset(data, 0, rlsp_buffer_size);
         this->shader_params.light_data.unmap_memory(data);
      }
   }
   void swap_chain_image::_setup_command_buffers() {
      this->command_buffers.setup(*this->owner);
      #if _DEBUG
         this->owner->set_debug_object_name(this->command_buffers.main_shadow.handle, QString("Swap Chain Image %1: Command Buffer: Sun Shadows").arg(this->my_index).toStdString());
         this->owner->set_debug_object_name(this->command_buffers.main.handle, QString("Swap Chain Image %1: Command Buffer: Main").arg(this->my_index).toStdString());
         this->owner->set_debug_object_name(this->command_buffers.fps.handle, QString("Swap Chain Image %1: Command Buffer: FPS/UI").arg(this->my_index).toStdString());
         this->owner->set_debug_object_name(this->command_buffers.finish.handle, QString("Swap Chain Image %1: Command Buffer: Finish").arg(this->my_index).toStdString());
      #endif
      this->command_buffers_invalid = true;
   }
   
   void swap_chain_image::setup_descriptor_sets() {
      this->descriptor_sets.allocate_all(*this->owner);
      //
      // Update descriptor sets for overlays that only need an initial update:
      //
      this->overlays.world_axes.initialize_descriptor_sets(*this);
      //
      // Standard render pass: sun shadow texture:
      //
      {
         auto image_info = VkDescriptorImageInfo{
            .sampler     = this->owner->canvas.sun_shadow.sampler,
            .imageView   = this->owner->canvas.sun_shadow.map.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         };
         auto write_info = VkWriteDescriptorSet{
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = this->descriptor_sets.standard,
            .dstBinding      = 2, // this should match the binding value in the shader
            .dstArrayElement = 0,
            .descriptorCount = (uint32_t)1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo      = &image_info,
         };
         vkUpdateDescriptorSets(this->owner->logical_device, (uint32_t)1, &write_info, 0, nullptr);
      }
      //
      // OIT:
      //
      {
         auto image_info_accumulator = VkDescriptorImageInfo{
            .sampler     = VK_NULL_HANDLE,
            .imageView   = this->owner->canvas.oit.accumulator.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         };
         auto image_info_reveal = VkDescriptorImageInfo{
            .sampler     = VK_NULL_HANDLE,
            .imageView   = this->owner->canvas.oit.reveal.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         };
         auto write_info = std::array{
            VkWriteDescriptorSet{
               .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
               .dstSet          = this->descriptor_sets.oit_composite,
               .dstBinding      = 0, // this should match the binding value in the shader
               .dstArrayElement = 0,
               .descriptorCount = (uint32_t)1,
               .descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
               .pImageInfo      = &image_info_accumulator,
            },
            VkWriteDescriptorSet{
               .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
               .dstSet          = this->descriptor_sets.oit_composite,
               .dstBinding      = 1, // this should match the binding value in the shader
               .dstArrayElement = 0,
               .descriptorCount = (uint32_t)1,
               .descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
               .pImageInfo      = &image_info_reveal,
            },
         };
         vkUpdateDescriptorSets(this->owner->logical_device, (uint32_t)write_info.size(), write_info.data(), 0, nullptr);
      }
   }
   void swap_chain_image::teardown_descriptor_sets() {
      vkFreeDescriptorSets(this->owner->logical_device, this->owner->descriptor_pool, this->descriptor_sets.list.size(), this->descriptor_sets.list.data());
   }

   void swap_chain_image::teardown() {
      if (!this->owner)
         return;
      this->image.teardown();
      //
      this->overlays.fps.teardown_atlas();
      //
      this->current_fence_handle = VK_NULL_HANDLE;
      this->invalidate_all_command_buffers();
   }

   void swap_chain_image::draw(frame_in_flight& fif) {
      this->_hook_to_frame(fif);
      //
      this->_update_shader_global_scene_state();
      this->_update_shader_lights_data_buffer();
      this->_update_shader_object_data_buffer();
      this->_update_shader_texture_descriptors(); // can invalidate command buffers, so must run before we check whether command buffers need refilling
      {
         auto& fps = this->overlays.fps;
         {
            auto delta = this->owner->last_frame_time();
            if (delta) {
               fps.set_value(decltype(delta)(1) / delta);
            } else {
               //
               // Instantaneous frame; dividing would be a division by zero. Just use the 
               // max possible FPS.
               //
               fps.set_value(vulkanDK::overlays::fps::max_value);
            }
         }
         //
         bool needs_re_record = false;
         if (fps.needs_atlas_update()) {
            fps.generate_atlas(*this->owner, *this);
            needs_re_record = true;
         }
         if (fps.needs_geometry_update()) {
            fps.update_geometry(*this->owner);
         }
         //
         if (!needs_re_record)
            needs_re_record = this->overlays.world_axes.needs_redraw(); // TODO: separate this from FPS redraw
         //
         if (needs_re_record && !this->command_buffers_invalid) { // refilling all command buffers also refills the buffer for this overlay
            this->_refill_fps_overlay_command_buffer();
         }
      }
      if (this->command_buffers_invalid) {
         //
         // The command buffer must render to the right framebuffer. Framebuffers are per 
         // swap chain image, so if frames-in-flight are NOT per swap chain image, then 
         // we need to redo the command buffers every frame so that they actually target 
         // the right framebuffer at any given moment.
         //
         this->_refill_command_buffers();
      }
      //
      // Submit our command buffers.
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
      std::vector<VkCommandBuffer> cb_handles;
      {
         auto& list = this->command_buffers.list;
         auto  size = list.size();
         cb_handles.resize(size);
         for (size_t i = 0; i < size; ++i)
            cb_handles[i] = list[i].handle;
      }
      auto wait_semaphores   = std::array{ fif.semaphores.image_available };
      auto signal_semaphores = std::array{ fif.semaphores.render_finished };
      VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
      auto submit_info = VkSubmitInfo{
         .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
         .waitSemaphoreCount   = wait_semaphores.size(),
         .pWaitSemaphores      = wait_semaphores.data(),
         .pWaitDstStageMask    = waitStages,
         .commandBufferCount   = (uint32_t)cb_handles.size(),
         .pCommandBuffers      = cb_handles.data(),
         .signalSemaphoreCount = signal_semaphores.size(),
         .pSignalSemaphores    = signal_semaphores.data(),
      };
      vkResetFences(this->owner->logical_device, 1, &fif.fence); // set the fence to unsignalled; vkWaitForFences calls will wait for it to be signalled
      if (vkQueueSubmit(this->owner->queues.graphics.handle, 1, &submit_info, fif.fence) != VK_SUCCESS) {
         throw std::runtime_error("failed to submit draw command buffer!");
      }
   }

   void swap_chain_image::invalidate_all_command_buffers() {
      this->command_buffers_invalid = true;
   }
   
   void swap_chain_image::_hook_to_frame(frame_in_flight& fif) {
      auto& handle = this->current_fence_handle;
      //
      // It may be the case that the last frame  to use this swap chain image is still 
      // drawing to it.  We can wait on that  frame's fence in order  to know when its 
      // command buffers have finished executing.
      //
      if (handle != VK_NULL_HANDLE) {
         vkWaitForFences(this->owner->logical_device, 1, &handle, VK_TRUE, UINT64_MAX);
      }
      //
      // Mark our swap chain  image as being in use for the new  frame and its command 
      // buffers.
      //
      handle = fif.fence;
   }
   //
   scene& swap_chain_image::get_scene() {
      assert(this->owner);
      return this->owner->scene;
   }
   void swap_chain_image::_update_shader_global_scene_state() {
      auto& scene = this->get_scene();
      auto& src   = scene.global_state;
      auto& dst   = this->shader_params.uniform;
      //
      void* data = dst.map_memory();
      memcpy(data, &src, sizeof(src));
      dst.unmap_memory(data);
   }
   void swap_chain_image::_update_shader_lights_data_buffer() {
      using entry_type = rendered_light::shader_parameters;
      constexpr auto entry_size = sizeof(entry_type);

      constexpr bool map_only_what_is_necessary = false; // useless in VMA

      auto& scene  = this->get_scene();
      auto& buffer = this->shader_params.light_data;
      //
      auto& list   = scene.lights;
      auto  count  = list.size();
      assert(count <= config::max_lights_in_scene);
      VkDeviceSize size = count * entry_size;
      //
      size_t first_dirty = 0;
      size_t last_dirty  = 0;
      bool   any_dirty   = false;
      if constexpr (map_only_what_is_necessary) {
         for (size_t i = 0; i < count; ++i) {
            auto& item = list[i];
            switch (item.life_state) {
               case scene_frame_item_state::empty:
                  continue;
               //
               // Unlike with rendered meshes, we actually do want to pass updated data for a 
               // rendered light that is pending deletion: we want to set its radius and color 
               // to zero and black. This is to avoid requiring the shader to check an "alive" 
               // bool on each light and branch; setting the light to zero and black is pretty 
               // much a branchless no-op.
               //
            }
            if (item.handled_frames.is_up_to_date(this->my_index))
               continue;
            if (!any_dirty) {
               first_dirty = i;
               any_dirty   = true;
            }
            last_dirty = i;
         }
      } else {
         for (size_t i = 0; i < count; ++i) {
            auto& item = list[i];
            switch (item.life_state) {
               case scene_frame_item_state::empty:
                  continue;
               case scene_frame_item_state::pending_delete:
                  item.handled_frames.set_up_to_date(this->my_index);
                  continue;
            }
            if (item.handled_frames.is_up_to_date(this->my_index))
               continue;
            first_dirty = i;
            any_dirty   = true;
            break;
         }
      }
      //
      if (any_dirty) {
         entry_type* data = nullptr;
         if constexpr (map_only_what_is_necessary) {
            VkDeviceSize offset = first_dirty * entry_size;
            VkDeviceSize length = (last_dirty - first_dirty + 1) * entry_size;
            data = (entry_type*)buffer.map_memory(offset, length);
            for (size_t i = first_dirty; i <= last_dirty; ++i) {
               auto& item = list[i];
               if (!item.active())
                  continue;
               if (item.handled_frames.is_up_to_date(this->my_index))
                  continue;
               auto& src = list[i].shader_params;
               auto& dst = data[i - first_dirty];
               memcpy(&dst, &src, entry_size);
               //
               item.handled_frames.set_up_to_date(this->my_index);
            }
         } else {
            data = (entry_type*)buffer.map_memory();
            for (size_t i = first_dirty; i < count; ++i) {
               auto& item = list[i];
               if (!item.active())
                  continue;
               if (item.handled_frames.is_up_to_date(this->my_index))
                  continue;
               auto& src = list[i].shader_params;
               auto& dst = data[i];
               memcpy(&dst, &src, entry_size);
               //
               item.handled_frames.set_up_to_date(this->my_index);
            }
         }
         buffer.unmap_memory(data);
      }
   }
   void swap_chain_image::_update_shader_object_data_buffer() {
      using entry_type = rendered_mesh::shader_parameters;
      constexpr auto entry_size = sizeof(entry_type);

      constexpr bool map_only_what_is_necessary = false; // useless in VMA

      auto& scene  = this->get_scene();
      auto& buffer = this->shader_params.object_data;
      //
      auto& ro     = scene.meshes;
      auto  count  = ro.size();
      assert(count <= config::max_rendered_meshes);
      VkDeviceSize size = count * entry_size;
      //
      size_t first_dirty = 0;
      size_t last_dirty  = 0;
      bool   any_dirty   = false;
      if constexpr (map_only_what_is_necessary) {
         for (size_t i = 0; i < count; ++i) {
            auto& item = ro[i];
            switch (item.life_state) {
               case scene_frame_item_state::empty:
                  continue;
               case scene_frame_item_state::pending_delete:
                  item.handled_frames.set_up_to_date(this->my_index);
                  continue;
            }
            if (item.handled_frames.is_up_to_date(this->my_index))
               continue;
            if (!any_dirty) {
               first_dirty = i;
               any_dirty   = true;
            }
            last_dirty = i;
         }
      } else {
         for (size_t i = 0; i < count; ++i) {
            auto& item = ro[i];
            switch (item.life_state) {
               case scene_frame_item_state::empty:
                  continue;
               case scene_frame_item_state::pending_delete:
                  item.handled_frames.set_up_to_date(this->my_index);
                  continue;
            }
            if (item.handled_frames.is_up_to_date(this->my_index))
               continue;
            first_dirty = i;
            any_dirty   = true;
            break;
         }
      }
      //
      if (any_dirty) {
         entry_type* data = nullptr;
         if constexpr (map_only_what_is_necessary) {
            VkDeviceSize offset = first_dirty * entry_size;
            VkDeviceSize length = (last_dirty - first_dirty + 1) * entry_size;
            data = (entry_type*)buffer.map_memory(offset, length);
            for (size_t i = first_dirty; i <= last_dirty; ++i) {
               auto& item = ro[i];
               if (!item.active())
                  continue;
               if (item.handled_frames.is_up_to_date(this->my_index))
                  continue;
               auto& src = ro[i].shader_params;
               auto& dst = data[i - first_dirty];
               memcpy(&dst, &src, entry_size);
               //
               item.handled_frames.set_up_to_date(this->my_index);
            }
         } else {
            data = (entry_type*)buffer.map_memory();
            for (size_t i = first_dirty; i < count; ++i) {
               auto& item = ro[i];
               if (!item.active())
                  continue;
               if (item.handled_frames.is_up_to_date(this->my_index))
                  continue;
               auto& src = ro[i].shader_params;
               auto& dst = data[i];
               memcpy(&dst, &src, entry_size);
               //
               item.handled_frames.set_up_to_date(this->my_index);
            }
         }
         buffer.unmap_memory(data);
      }
   }
   void swap_chain_image::_update_shader_texture_descriptors() {
      auto&    scene = this->get_scene();
      auto&    list  = scene.textures;
      uint32_t size  = list.size();
      //
      bool  needs_null_texture = this->owner->needs_null_texture();
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
      for (uint32_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.handled_frames.is_up_to_date(this->my_index))
            continue;
         if (item.empty()) // deleted texture
            continue;
         item.handled_frames.set_up_to_date(this->my_index); // this is only good for single-threaded; for multi-threaded we're gonna need to do this AFTER the descriptor writes go through
         auto view = item.content.view;
         if (item.pending_delete()) {
            if (needs_null_texture) {
               view = null_texture.view;
               assert(view != VK_NULL_HANDLE && "Null descriptor handles aren't supported, but we never set up our null texture!");
            } else {
               view = VK_NULL_HANDLE;
            }
         }
         if constexpr (debug_log_scene_object_lifetimes) {
            qDebug("[vulkanDK::swap_chain_image::_update_shader_texture_descriptors] Updating scene texture %u (deleted: %u).", i, item.pending_delete());
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
               VkDescriptorImageInfo{
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
      auto& target_set = this->descriptor_sets.standard;
      //
      std::vector<VkWriteDescriptorSet> write_info(writes.size());
      for (size_t i = 0; i < writes.size(); ++i) {
         auto& src  = writes[i];
         auto& info = writes[i].entries;
         //
         write_info[i] = VkWriteDescriptorSet{ // texture array
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = target_set,
            .dstBinding      = 5, // this should match the binding value in the shader
            .dstArrayElement = src.start,
            .descriptorCount = (uint32_t)info.size(),
            .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo      = info.data(),
         };
      }
      vkUpdateDescriptorSets(this->owner->logical_device, (uint32_t)write_info.size(), write_info.data(), 0, nullptr);
      {
         //
         // And sun shadows...
         //
         auto& target_set = this->descriptor_sets.sun_shadows;
         for (auto& item : write_info) {
            item.dstSet     = target_set;
            item.dstBinding = 3;
         }
         vkUpdateDescriptorSets(this->owner->logical_device, (uint32_t)write_info.size(), write_info.data(), 0, nullptr);
      }
      //
      // Updating a descriptor set will invalidate any command buffers using it; they must 
      // be reset and their queue regenerated:
      //
      this->invalidate_all_command_buffers();
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::swap_chain_image::_update_shader_texture_descriptors] Invalidated command buffers.");
      }
   }
   void swap_chain_image::_refill_command_buffers() {
      this->command_buffers_invalid = false;
      //
      auto& scene = this->owner->scene;
      {  // Sun shadows
         auto& command_buffer = this->command_buffers.main_shadow;
         auto  command_handle = command_buffer.handle;
         //
         command_buffer.reset(0);
         if (command_buffer.top_level_begin(0) != VK_SUCCESS) {
            throw std::runtime_error("[vulkanDK::swap_chain_image::_refill_command_buffers] Failed to begin recording command buffer (sun shadows).");
         }
         //
         command_buffer.begin_render_pass(
            *this->owner->render_passes_by_name.main_shadow,
            this->owner->canvas.sun_shadow.framebuffer,
            {
               .offset = { 0, 0 },
               .extent = { config::sun_shadow_map_resolution_x, config::sun_shadow_map_resolution_y },
            },
            std::array{
               VkClearValue{ .depthStencil = { config::use_inverted_shadow_map ? 0.0 : 1.0, 0 } },
            },
            VK_SUBPASS_CONTENTS_INLINE
         );
         {
            const shader* shader   = this->owner->get_shader(surface_renderer::sun_shadow_shader_id);
            assert(shader);
            const auto&   material = shader->material;
            command_buffer.bind_material_and_descriptors(material, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, std::array{ this->descriptor_sets.sun_shadows });
            //
            VkDeviceSize offset = 0;
            for (size_t j = 0; j < scene.meshes.size(); ++j) {
               auto& ro  = scene.meshes[j];
               auto& vib = ro.vertex_and_index_buffer;
               if (!ro.active())
                  continue;

               auto pc = ro.push_params;
               pc.object_index         = (int32_t)j;
               pc.texture_index        = (int32_t)ro.texture_indices.diffuse;
               pc.texture_normal_index = (int32_t)ro.texture_indices.normals;
               //
               command_buffer.set_pipeline_push_constant(material, VK_SHADER_STAGE_VERTEX_BIT, pc);
               ro.draw_call(command_handle);
            }
         }
         vkCmdEndRenderPass(command_handle);
         if (command_buffer.finish() != VK_SUCCESS) {
            throw std::runtime_error("[vulkanDK::swap_chain_image::_refill_command_buffers] Failed to record a command buffer (sun shadows).");
         }
      }
      //
      // Normal objects:
      //
      bool can_do_alpha = this->owner->can_do_alpha();
      //
      {  // Scene objects
         auto& command_buffer = this->command_buffers.main;
         auto  command_handle = command_buffer.handle;
         //
         command_buffer.reset(0);
         if (command_buffer.top_level_begin(0) != VK_SUCCESS) {
            throw std::runtime_error("[vulkanDK::swap_chain_image::_refill_command_buffers] Failed to begin recording command buffer.");
         }
         this->owner->canvas.color.transition_layout(command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
         //
         command_buffer.begin_render_pass(
            *this->owner->render_passes_by_name.main,
            this->owner->canvas.main_framebuffer,
            {
               .offset = { 0, 0 },
               .extent = this->owner->surface_extent,
            },
            std::array{
               VkClearValue{ .color = { 0, 0, 0, 1 } }, // set framebuffer to black
               VkClearValue{ .depthStencil = { config::use_inverted_depth ? 0.0 : 1.0, 0} }, // depth attachment uses VK_ATTACHMENT_LOAD_OP_CLEAR; this is the depth range to celar with
            },
            VK_SUBPASS_CONTENTS_INLINE
         );
         {
            //
            // We'd want to pre-sort objects by material, and re-bind descriptor sets and pipelines 
            // with each new material.
            //
            const shader* shader = this->owner->get_shader(surface_renderer::main_shader_id);
            assert(shader);
            const auto& material = shader->material;
            command_buffer.bind_material_and_descriptors(material, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, std::array{ this->descriptor_sets.standard });
            //
            for (size_t j = 0; j < scene.meshes.size(); ++j) {
               auto& ro = scene.meshes[j];
               auto& vib = ro.vertex_and_index_buffer;
               if (!ro.active())
                  continue;
               if (can_do_alpha && (ro.mesh_flags & rendered_mesh::mesh_flag::requires_oit))
                  continue;
               
               auto pc = ro.push_params;
               pc.object_index         = (int32_t)j;
               pc.texture_index        = (int32_t)ro.texture_indices.diffuse;
               pc.texture_normal_index = (int32_t)ro.texture_indices.normals;
               //
               command_buffer.set_pipeline_push_constant(material, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, pc);
               ro.draw_call(command_handle);
            }
         }
         command_buffer.end_render_pass();
         //
         if (can_do_alpha) {
            assert(this->owner->render_passes_by_name.main_oit);
            //
            // OIT passes:
            //
            command_buffer.begin_render_pass(
               *this->owner->render_passes_by_name.main_oit,
               this->owner->canvas.oit.framebuffer,
               {
                  .offset = { 0, 0 },
                  .extent = this->owner->surface_extent,
               },
               std::array{
                  VkClearValue{ .color = { 0, 0, 0, 0 } }, // accumulator
                  VkClearValue{ .color = { 1, 0, 0, 0 } }, // reveal
               },
               VK_SUBPASS_CONTENTS_INLINE
            );
            {
               const shader* shader = this->owner->get_shader(surface_renderer::main_shader_oit_color_id);
               assert(shader);
               const auto& material = shader->material;
               command_buffer.bind_material_and_descriptors(material, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, std::array{ this->descriptor_sets.standard });
               for (size_t j = 0; j < scene.meshes.size(); ++j) {
                  auto& ro = scene.meshes[j];
                  auto& vib = ro.vertex_and_index_buffer;
                  if (!ro.active())
                     continue;
                  if (!(ro.mesh_flags & rendered_mesh::mesh_flag::requires_oit))
                     continue;

                  auto pc = ro.push_params;
                  pc.object_index         = (int32_t)j;
                  pc.texture_index        = (int32_t)ro.texture_indices.diffuse;
                  pc.texture_normal_index = (int32_t)ro.texture_indices.normals;
                  //
                  command_buffer.set_pipeline_push_constant(material, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, pc);
                  ro.draw_call(command_handle);
               }
            }
            vkCmdNextSubpass(command_handle, VK_SUBPASS_CONTENTS_INLINE);
            {
               const shader* shader = this->owner->get_shader(surface_renderer::oit_composite_shader_id);
               assert(shader);
               const auto& material = shader->material;
               command_buffer.bind_material_and_descriptors(material, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, std::array{ this->descriptor_sets.oit_composite });
               vkCmdDraw(command_handle, 3, 1, 0, 0);
            }
            command_buffer.end_render_pass();
         }
         //
         // Done!
         //
         if (command_buffer.finish() != VK_SUCCESS) {
            throw std::runtime_error("[vulkanDK::swap_chain_image::_refill_command_buffers] Failed to record a command buffer.");
         }
      }
      //
      this->_refill_fps_overlay_command_buffer();
      //
      {  // Finish
         auto& command_buffer = this->command_buffers.finish;
         auto  command_handle = command_buffer.handle;
         //
         command_buffer.reset(0);
         if (command_buffer.top_level_begin(0) != VK_SUCCESS) {
            throw std::runtime_error("[vulkanDK::swap_chain_image::_refill_command_buffers] Failed to begin recording command buffer (finish).");
         }
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

            this->owner->canvas.color.transition_layout(command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT);
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
         this->owner->canvas.color.transition_layout(command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
         if (command_buffer.finish() != VK_SUCCESS) {
            throw std::runtime_error("[vulkanDK::swap_chain_image::_refill_command_buffers] Failed to record a command buffer (finish).");
         }
      }
   }
   void swap_chain_image::_refill_fps_overlay_command_buffer() {
      auto& command_buffer = this->command_buffers.fps;
      auto  command_handle = command_buffer.handle;
      //
      command_buffer.reset(0);
      if (command_buffer.top_level_begin(0) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::swap_chain_image::_refill_fps_overlay_command_buffer] Failed to begin recording UI command buffer.");
      }
      //
      this->overlays.fps.commands_pre_pass(command_handle); // commands that must run before vkCmdBeginRenderPass
      this->overlays.world_axes.commands_pre_pass(command_handle); // commands that must run before vkCmdBeginRenderPass
      //
      command_buffer.begin_render_pass(
         *this->owner->render_passes_by_name.ui,
         this->owner->canvas.main_framebuffer,
         {
            .offset = { 0, 0 },
            .extent = this->owner->surface_extent,
         },
         std::array{
            VkClearValue{ .color        = { 0, 0, 0, 0 } },
            VkClearValue{ .depthStencil = { 1.0, 0 } }, // don't apply inverted depth here; that's per projection matrix
         },
         VK_SUBPASS_CONTENTS_INLINE
      );
      {
         const shader* shader = this->owner->get_shader(vulkanDK::overlays::fps::shader_id);
         if (shader) {
            command_buffer.bind_material_and_descriptors(shader->material, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, std::array{ this->descriptor_sets.fps });
            this->overlays.fps.draw_call(command_handle);
         }
      }
      {
         const shader* shader = this->owner->get_shader(vulkanDK::overlays::world_axes::shader_id);
         if (shader) {
            command_buffer.bind_material_and_descriptors(shader->material, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, std::array{ this->descriptor_sets.world_axes });
            this->overlays.world_axes.draw_call(command_handle);
         }
      }
      command_buffer.end_render_pass();
      if (command_buffer.finish() != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::swap_chain_image::_refill_fps_overlay_command_buffer] Failed to record the UI command buffer.");
      }
   }
}