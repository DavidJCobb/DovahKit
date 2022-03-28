#include "frame_in_flight.h"
#include <cassert>
#include "exceptions.h"
#include "surface_renderer.h"
#include "config/scene_limits.h"
#include "config/shadow_maps.h"
#include "config/use_inverted_depth.h"

namespace {
   static constexpr bool debug_log_scene_object_lifetimes = false;
}

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

   void frame_in_flight::setup(surface_renderer& sr, size_t my_index) {
      this->owner    = &sr;
      this->my_index = my_index;
      //
      this->_setup_semaphores();
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
   void frame_in_flight::_setup_shader_parameter_buffers() {
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
   void frame_in_flight::_setup_command_buffers() {
      this->command_buffers.setup(*this->owner);
      #if _DEBUG
         this->owner->set_debug_object_name(this->command_buffers.main_shadow.handle, QString("Frame-in-Flight %1: Command Buffer: Sun Shadows").arg(this->my_index).toStdString());
         this->owner->set_debug_object_name(this->command_buffers.main.handle,        QString("Frame-in-Flight %1: Command Buffer: Main").arg(this->my_index).toStdString());
         this->owner->set_debug_object_name(this->command_buffers.fps.handle,         QString("Frame-in-Flight %1: Command Buffer: FPS/UI").arg(this->my_index).toStdString());
      #endif
      this->command_buffers_invalid = true;
   }
   
   void frame_in_flight::setup_descriptor_sets() {
      this->descriptor_sets.allocate_all(*this->owner);
      //
      // Update descriptor sets for overlays that only need an initial update:
      //
      this->overlays.world_axes.initialize_descriptor_sets(*this);
   }

   void frame_in_flight::pre_resize() {
      this->invalidate_all_command_buffers();
      this->teardown_descriptor_sets();
   }
   void frame_in_flight::post_resize() {
      this->setup_descriptor_sets();
      this->overlays.fps.handle_resize(*this->owner, *this);
      this->overlays.world_axes.handle_resize(*this->owner);
   }

   void frame_in_flight::teardown_descriptor_sets() {
      vkFreeDescriptorSets(this->owner->logical_device, this->owner->descriptor_pool, this->descriptor_sets.list.size(), this->descriptor_sets.list.data());
   }

   void frame_in_flight::teardown() {
      if (!this->owner)
         return;
      this->overlays.fps.teardown_atlas();
      this->invalidate_all_command_buffers();
   }

   void frame_in_flight::record_draw_commands() {
      this->_update_shader_global_scene_state();
      this->_update_shader_lights_data_buffer();
      this->_update_shader_object_data_buffer();
      this->_update_shader_texture_descriptors(); // can invalidate command buffers, so must run before we check whether command buffers need refilling
      this->owner->scene.update_light_shadows(*this);
      {
         auto& fps = this->overlays.fps;
         {
            auto delta = this->owner->last_frame_time();
            if (delta) {
               fps.set_value(decltype(delta)(1) / delta);
            } else {
               //
               // Instantaneous frame; dividing would be a  division by zero. Refer to documentation on 
               // how we measure FPS, but basically, it's best to just skip measuring this frame.
               //
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
   }

   void frame_in_flight::invalidate_all_command_buffers() {
      this->command_buffers_invalid = true;
   }
   
   scene& frame_in_flight::get_scene() {
      assert(this->owner);
      return this->owner->scene;
   }
   void frame_in_flight::_update_shader_global_scene_state() {
      auto& scene = this->get_scene();
      auto& src   = scene.global_state;
      auto& dst   = this->shader_params.uniform;
      //
      void* data = dst.map_memory();
      memcpy(data, &src, sizeof(src));
      dst.unmap_memory(data);
   }
   void frame_in_flight::_update_shader_lights_data_buffer() {
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
   void frame_in_flight::_update_shader_object_data_buffer() {
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
   void frame_in_flight::_update_shader_texture_descriptors() {
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
            qDebug("[vulkanDK::frame_in_flight::_update_shader_texture_descriptors] Updating scene texture %u (deleted: %u).", i, item.pending_delete());
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
            .dstBinding      = 6, // this should match the binding value in the shader
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
      {
         //
         // And light shadows...
         //
         auto& target_set = this->descriptor_sets.light_shadows;
         for (auto& item : write_info) {
            item.dstSet     = target_set;
            item.dstBinding = 4;
         }
         vkUpdateDescriptorSets(this->owner->logical_device, (uint32_t)write_info.size(), write_info.data(), 0, nullptr);
      }
      //
      // Updating a descriptor set will invalidate any command buffers using it; they must 
      // be reset and their queue regenerated:
      //
      this->invalidate_all_command_buffers();
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::frame_in_flight::_update_shader_texture_descriptors] Invalidated command buffers.");
      }
   }

   namespace {
      rendered_mesh::push_constant _make_push_constant_for(const rendered_mesh& ro, size_t object_index) {
         auto pc = ro.push_params;
         pc.object_index         = (int32_t)object_index;
         pc.texture_index        = (int32_t)ro.texture_indices.diffuse;
         pc.texture_normal_index = (int32_t)ro.texture_indices.normals;
         return pc;
      }

      template<typename Pred, typename Prior> requires requires(const rendered_mesh& ro, Pred&& predicate, Prior&& before_draw) {
         { predicate(ro) } -> std::same_as<bool>;
         { before_draw(ro) };
      }
      void _draw_objects(scene& scene, command_buffer& command_buffer, const shader& shader, Pred&& predicate, Prior&& before_draw) {
         auto   command_handle     = command_buffer.handle;
         auto&  material           = shader.material;
         size_t first_double_sided = std::string::npos;
         //
         for (size_t j = 0; j < scene.meshes.size(); ++j) {
            auto& ro = scene.meshes[j];
            if (!ro.active())
               continue;
            if (!predicate(ro))
               continue;
            if (ro.mesh_flags & rendered_mesh::mesh_flag::double_sided) {
               first_double_sided = j;
               continue;
            }

            before_draw(ro);

            command_buffer.set_pipeline_push_constant(material, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, _make_push_constant_for(ro, j));
            ro.draw_call(command_handle);
         }
         //
         if (first_double_sided != std::string::npos) {
            auto* variant = shader.get_variant({
               .face_cull_mode = VK_CULL_MODE_NONE,
            });
            assert(variant);
            vkCmdBindPipeline(command_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, variant->handle);
            //
            for (size_t j = 0; j < scene.meshes.size(); ++j) {
               auto& ro = scene.meshes[j];
               if (!ro.active())
                  continue;
               if (!(ro.mesh_flags & rendered_mesh::mesh_flag::double_sided))
                  continue;
               if (!predicate(ro))
                  continue;

               before_draw(ro);
               
               command_buffer.set_pipeline_push_constant(material, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, _make_push_constant_for(ro, j));
               ro.draw_call(command_handle);
            }
         }
      }
   }
   void frame_in_flight::_refill_command_buffers() {
      this->command_buffers_invalid = false;
      //
      auto& scene = this->owner->scene;
      {  // Sun shadows
         auto& command_buffer = this->command_buffers.main_shadow;
         auto  command_handle = command_buffer.handle;
         //
         command_buffer.reset(0);
         if (auto result = command_buffer.top_level_begin(0); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::_refill_command_buffers] Failed to begin recording command buffer (sun shadows).");
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
            _draw_objects(
               scene, command_buffer, *shader,
               [](const rendered_mesh& ro) {
                  return (ro.mesh_flags & rendered_mesh::mesh_flag::cast_shadows) != 0;
               },
               [](const rendered_mesh& ro) {}
            );
         }
         vkCmdEndRenderPass(command_handle);
         if (auto result = command_buffer.finish(); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::_refill_command_buffers] Failed to record a command buffer (sun shadows).");
         }
      }
      //
      {  // Point light shadows
         auto& command_buffer = this->command_buffers.main_shadow_placed;
         auto  command_handle = command_buffer.handle;
         //
         command_buffer.reset(0);
         if (auto result = command_buffer.top_level_begin(0); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::_refill_command_buffers] Failed to begin recording command buffer (light shadows).");
         }
         //
         static_assert(false, "TODO: We want to render one cubemap face at a time. This means we should render to an R-only color image and a depth buffer, and then copy from that to a face in our final cubemap.");
            static_assert(false, "...at least, that's what Sascha Willems' example does. But why do we need an R-only image when we're already doing a depth buffer? Why not just the depth buffer?");
         static_assert(false, "TODO: If possible, we should use multi-view if enabled; see: https://blog/anishbhobe.site/vulkan-render-to-cubemaps-using-multiview/; else fall back to one face at a time");
         command_buffer.begin_render_pass(
            *this->owner->render_passes_by_name.main_shadow_placed,
            this->owner->canvas.light_shadows.framebuffer,
            {
               .offset = { 0, 0 },
               .extent = { config::light_shadow_map_resolution_x, config::light_shadow_map_resolution_y },
            },
            std::array{
               VkClearValue{ .depthStencil = { config::use_inverted_shadow_map ? 0.0 : 1.0, 0 } },
               VkClearValue{ .depthStencil = { config::use_inverted_shadow_map ? 0.0 : 1.0, 0 } },
               VkClearValue{ .depthStencil = { config::use_inverted_shadow_map ? 0.0 : 1.0, 0 } },
               VkClearValue{ .depthStencil = { config::use_inverted_shadow_map ? 0.0 : 1.0, 0 } },
               VkClearValue{ .depthStencil = { config::use_inverted_shadow_map ? 0.0 : 1.0, 0 } },
               VkClearValue{ .depthStencil = { config::use_inverted_shadow_map ? 0.0 : 1.0, 0 } },
               VkClearValue{ .depthStencil = { config::use_inverted_shadow_map ? 0.0 : 1.0, 0 } },
               VkClearValue{ .depthStencil = { config::use_inverted_shadow_map ? 0.0 : 1.0, 0 } },
            },
            VK_SUBPASS_CONTENTS_INLINE
         );
         static_assert(false, "TODO: update the clear values");
         //
         for (size_t i = 0; i < surface_renderer::shadow_caster_count; ++i) {
            const auto light_index = scene.global_state.shadow_caster_index[i];
            if (light_index < 0 || light_index >= scene.lights.size()) {
               vkCmdNextSubpass(command_handle, VK_SUBPASS_CONTENTS_INLINE);
               if (i + 1 < surface_renderer::shadow_caster_count) // ensure we don't advance past the last subpass
                  vkCmdNextSubpass(command_handle, VK_SUBPASS_CONTENTS_INLINE);
               continue;
            }
            const auto& light = scene.lights[light_index];
            //
            auto id = surface_renderer::light_shadow_map_shader_base_id;
            id.bytes[6] = '0' + i;
            //
            for (int j = 0; j < 6; ++j) { // for each cubemap face
               id.bytes[7] = '0' + j;
               {
                  const shader* shader   = this->owner->get_shader(id);
                  assert(shader);
                  const auto&   material = shader->material;
                  command_buffer.bind_material_and_descriptors(material, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, std::array{ this->descriptor_sets.light_shadows });
                  //
                  _draw_objects(
                     scene, command_buffer, *shader,
                     [](const rendered_mesh& ro) {
                        return (ro.mesh_flags & rendered_mesh::mesh_flag::cast_shadows) != 0;
                     },
                     [](const rendered_mesh& ro) {}
                  );
               }
               if (j != 5 || i != surface_renderer::shadow_caster_count - 1)
                  vkCmdNextSubpass(command_handle, VK_SUBPASS_CONTENTS_INLINE);
            }
         }
         vkCmdEndRenderPass(command_handle);
         if (auto result = command_buffer.finish(); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::_refill_command_buffers] Failed to record a command buffer (light shadows).");
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
         if (auto result = command_buffer.top_level_begin(0); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::_refill_command_buffers] Failed to begin recording command buffer.");
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
         //
         bool last_was_decal   = false;
         auto set_decal_config = [command_handle, &last_was_decal](const rendered_mesh& ro) {
            bool current_is_decal = (ro.mesh_flags & rendered_mesh::mesh_flag::is_decal) != 0;
            if (current_is_decal != last_was_decal) {
               if (current_is_decal) {
                  //
                  // "Depth bias" in Vulkan is functionally equivalent to OpenGL's glPolygonOffset, and 
                  // can be used to apply a depth offset to triangle-based models when generating the 
                  // depth buffer and (I believe) when rendering it in general. This can prevent meshes 
                  // from Z-fighting even when they're coplanar, as would typically be the case for any 
                  // decal meshes.
                  //
                  constexpr auto base  = config::use_inverted_depth ? 1.25 : 1.25;
                  constexpr auto limit = 0.0; // no limit, for now; NOTE: requires a hardware feature
                  constexpr auto scale = config::use_inverted_depth ? 1.75 : 1.75;
                  //
                  // NOTE: Merely setting the depth bias parameters isn't enough; depth bias must actually 
                  // be enabled as well. You can enable it when defining the shader, with the parameters 
                  // set to zero initially; or, if you're using Vulkan 1.3+, you can flag "depth bias is 
                  // enabled" as a dynamic state parameter and then use vkCmdSetDepthBiasEnable here.
                  //
                  vkCmdSetDepthBias(command_handle, base, limit, scale);
               } else {
                  vkCmdSetDepthBias(command_handle, 0.0, 0.0, 0.0);
               }
               last_was_decal = current_is_decal;
            }
         };
         vkCmdSetDepthBias(command_handle, 0.0, 0.0, 0.0); // we have to set the initial state as well, so let's pick the value that matches (last_was_decal)
         {
            //
            // We'd want to pre-sort objects by material, and re-bind descriptor sets and pipelines 
            // with each new material.
            //
            const shader* shader;
            if (this->owner->debug.show_shadow_caster_depths == std::string::npos) {
               shader = this->owner->get_shader(surface_renderer::main_shader_id);
            } else {
               cobb::eight_cc id = "DBGLite0";
               id.bytes[7] += this->owner->debug.show_shadow_caster_depths;
               shader = this->owner->get_shader(id);
            }
            assert(shader);
            const auto& material = shader->material;
            command_buffer.bind_material_and_descriptors(material, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, std::array{ this->descriptor_sets.standard });
            //
            _draw_objects(
               scene, command_buffer, *shader,
               [can_do_alpha](const rendered_mesh& ro) {
                  return !(can_do_alpha && (ro.mesh_flags & rendered_mesh::mesh_flag::requires_oit));
               },
               [&set_decal_config](const rendered_mesh& ro) {
                  set_decal_config(ro);
               }
            );
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
               //
               _draw_objects(
                  scene, command_buffer, *shader,
                  [](const rendered_mesh& ro) {
                     return (ro.mesh_flags & rendered_mesh::mesh_flag::requires_oit) != 0;
                  },
                  [&set_decal_config](const rendered_mesh& ro) {
                     set_decal_config(ro);
                  }
               );
            }
            vkCmdNextSubpass(command_handle, VK_SUBPASS_CONTENTS_INLINE);
            //
            // Compositing:
            //
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
         if (last_was_decal) {
            vkCmdSetDepthBias(command_handle, 0.0, 0.0, 0.0);
         }
         //
         // Done!
         //
         if (auto result = command_buffer.finish(); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::_refill_command_buffers] Failed to record a command buffer.");
         }
      }
      //
      this->_refill_fps_overlay_command_buffer();
   }
   void frame_in_flight::_refill_fps_overlay_command_buffer() {
      auto& command_buffer = this->command_buffers.fps;
      auto  command_handle = command_buffer.handle;
      //
      command_buffer.reset(0);
      if (auto result = command_buffer.top_level_begin(0); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::frame_in_flight::_refill_fps_overlay_command_buffer] Failed to begin recording UI command buffer.");
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
      if (auto result = command_buffer.finish(); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::frame_in_flight::_refill_fps_overlay_command_buffer] Failed to record the UI command buffer.");
      }
   }
}