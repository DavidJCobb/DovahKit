#include "frame_in_flight.h"
#include <cassert>
#include "helpers/memset.h"
#include "compute_shader.h"
#include "exceptions.h"
#include "surface_renderer.h"
#include "config/scene_limits.h"
#include "config/shadow_maps.h"
#include "config/use_inverted_depth.h"
#include "helpers/extract_frustum_normals.h"

namespace {
   static constexpr bool debug_log_scene_object_lifetimes = false;
}

namespace vulkanDK {
   void frame_in_flight::indirect_draw_buffers::setup(surface_renderer& sr, const std::string& debug_name) {
      constexpr VkDeviceSize params_size = sizeof(VkDrawIndexedIndirectCommand) * config::max_rendered_meshes;
      this->params = sr.create_buffer(params_size,  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      //
      constexpr VkDeviceSize indices_size = sizeof(mesh_index_list::value_type) * config::max_rendered_meshes;
      this->mesh_indices.gpu  = sr.create_buffer(indices_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      this->mesh_indices.host = std::make_unique<mesh_index_list>();
      //this->mesh_indices_staging = sr.create_buffer(indices_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      //
      sr.set_debug_object_name(this->mesh_indices.gpu.handle, std::string("Buffer: IDB: Mesh Indices (") + debug_name + ")");
      sr.set_debug_object_name(this->params.handle, std::string("Buffer: IDB: Draw Params (") + debug_name + ")");
      //
      this->barriers.to_compute = {
         .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
         .pNext = nullptr,
         .srcAccessMask       = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
         .dstAccessMask       = VK_ACCESS_SHADER_WRITE_BIT,
         .srcQueueFamilyIndex = sr.queues.graphics.index,
         .dstQueueFamilyIndex = sr.queues.compute.index,
         .buffer = this->params.handle,
         .size   = VK_WHOLE_SIZE,
      };
      this->barriers.to_graphics = {
         .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
         .pNext = nullptr,
         .srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT,
         .dstAccessMask       = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
         .srcQueueFamilyIndex = sr.queues.compute.index,
         .dstQueueFamilyIndex = sr.queues.graphics.index,
         .buffer = this->params.handle,
         .size   = VK_WHOLE_SIZE,
      };
   }

   void frame_in_flight::indirect_draw_buffers::transfer_queue_ownership_to_compute(VkCommandBuffer command_handle) const {
      vkCmdPipelineBarrier(
         command_handle,
         VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
         0,
         0, nullptr,
         1, &this->barriers.to_compute,
         0, nullptr
      );
   }
   void frame_in_flight::indirect_draw_buffers::transfer_queue_ownership_to_graphics(VkCommandBuffer command_handle) const {
      vkCmdPipelineBarrier(
         command_handle,
         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
         VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
         0,
         0, nullptr,
         1, &this->barriers.to_graphics,
         0, nullptr
      );
   }


   frame_in_flight::frame_in_flight() {
      // Initialize these to `true` so that we build indirect draw data on startup. Failing to do this 
      // will result in any initial scene meshes failing to draw, and in us attempting thousands of 
      // draws on zero-vertex "meshes" that the FiF will think exists.
      this->state.scene_entity_draws_changed.for_each([](bool& flag) { flag = true; });
   }
   frame_in_flight::~frame_in_flight() {
      if (!this->owner) {
         assert(this->fences.graphics == VK_NULL_HANDLE && "The owning surface renderer should've torn down its frames-in-flight before their own destructor ran.");
         return;
      }
   }

   void frame_in_flight::setup(surface_renderer& sr, size_t my_index) {
      this->owner    = &sr;
      this->my_index = my_index;
      //
      this->_setup_semaphores();
      this->_setup_shader_parameter_buffers();
      this->_setup_command_buffers();
      this->idb_mesh_index_staging = sr.create_buffer(sizeof(indirect_draw_buffers::mesh_index_list), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
      this->idb_params_staging = sr.create_buffer(sizeof(VkDrawIndexedIndirectCommand) * config::max_rendered_meshes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
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
      if (auto result = vkCreateSemaphore(device, &semaphore_info, nullptr, &this->semaphores.compute_finished); result != VK_SUCCESS) {
         throw result_exception(result, "[frame_in_flight::_setup_semaphores] Failed to create frame-in-flight semaphore (compute-finished).");
      }
      if (auto result = vkCreateSemaphore(device, &semaphore_info, nullptr, &this->semaphores.graphics_finished); result != VK_SUCCESS) {
         throw result_exception(result, "[frame_in_flight::_setup_semaphores] Failed to create frame-in-flight semaphore (graphics-finished).");
      } else {
         auto& queue = this->owner->queues.graphics;
         //
         // We want the semaphore to be initially signalled, so that prior rendering stages can 
         // blindly wait on it instead of having to skip it on the first frame.
         //
         auto queue_handle = this->owner->queues.graphics.handle;
         auto submit_info  = VkSubmitInfo{
            .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount   = 0,
            .pWaitSemaphores      = nullptr,
            .pWaitDstStageMask    = 0,
            .commandBufferCount   = 0,
            .pCommandBuffers      = VK_NULL_HANDLE,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = &this->semaphores.graphics_finished,
         };
         if (auto result = vkQueueSubmit(queue_handle, 1, &submit_info, VK_NULL_HANDLE); result != VK_SUCCESS) {
            throw result_exception(result, "[frame_in_flight::_setup_semaphores] Failed to initially signal frame-in-flight semaphore (graphics-finished).");
         }
         vkQueueWaitIdle(queue_handle);
      }
      if (auto result = vkCreateSemaphore(device, &semaphore_info, nullptr, &this->semaphores.render_finished); result != VK_SUCCESS) {
         throw result_exception(result, "[frame_in_flight::_setup_semaphores] Failed to create frame-in-flight semaphore (render-finished).");
      }
      if (auto result = vkCreateFence(device, &fence_info, nullptr, &this->fences.graphics); result != VK_SUCCESS) {
         throw result_exception(result, "[frame_in_flight::_setup_semaphores] Failed to create graphics fence.");
      }
      if (auto result = vkCreateFence(device, &fence_info, nullptr, &this->fences.compute); result != VK_SUCCESS) {
         throw result_exception(result, "[frame_in_flight::_setup_semaphores] Failed to create compute fence.");
      }
   }
   void frame_in_flight::_setup_shader_parameter_buffers() {
      {
         constexpr VkDeviceSize buffer_size = sizeof(glm::vec4) * config::max_active_shadow_casters;
         this->shader_params.active_caster_positions = this->owner->create_buffer(buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
         this->owner->set_debug_object_name(this->shader_params.active_caster_positions.handle, QString("Buffer: FIF %1 Active Caster Position Buffer").arg(this->my_index).toStdString());
      }
      this->shader_params.scene_entity_frame_culling_data.for_each([this]<typename Entity>(buffer& buf) {
         constexpr VkDeviceSize buffer_size = scene_entities::initial_cap_for_type<Entity> * sizeof(Entity::frame_culling_data_type);
         buf = this->owner->create_buffer(buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
         this->owner->set_debug_object_name(
            buf.handle, QString("Buffer: FIF %1 Frame Culling Data Buffer (%s)")
               .arg(this->my_index)
               .arg(Entity::name_plural)
               .toStdString()
         );
      });
      //
      {
         constexpr VkDeviceSize buffer_size = sizeof(scene_global_state);
         this->shader_params.scene_data = this->owner->create_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
         this->owner->set_debug_object_name(this->shader_params.scene_data.handle, QString("Buffer: FIF %1 Scene Global State Buffer").arg(this->my_index).toStdString());
      }
      this->shader_params.scene_entity_frame_drawing_data.for_each([this]<typename Entity>(buffer& buf) {
         constexpr VkDeviceSize buffer_size = scene_entities::initial_cap_for_type<Entity> * sizeof(Entity::frame_drawing_data_type);
         buf = this->owner->create_buffer(buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
         this->owner->set_debug_object_name(
            buf.handle, QString("Buffer: FIF %1 Frame Drawing Data Buffer (%s)")
               .arg(this->my_index)
               .arg(Entity::name_plural)
               .toStdString()
         );
         //
         if constexpr (!Entity::is_drawn) {
            //
            // If the entity is not drawn, and has frame drawing data, then it's likely that 
            // the entity's "existence" is contingent on the content of its frame drawing data. 
            // Ergo, zero-initialize that data.
            // 
            // Rendered lights are an example of this.
            //
            auto* data = buf.map_memory();
            memset(data, 0, buffer_size);
            buf.unmap_memory(data);
         }
      });
      {
         constexpr VkDeviceSize buffer_size = surface_renderer::shadow_caster_count * (6 * sizeof(glm::mat4));
         this->shader_params.light_shadow_data = this->owner->create_buffer(buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
         this->owner->set_debug_object_name(this->shader_params.light_shadow_data.handle, QString("Buffer: FIF %1 Scene Shadow-Caster Data Buffer").arg(this->my_index).toStdString());
      }
      //
      // Indirect draw params:
      //
      {
         auto& idb = this->indirect_draw_commands;
         idb.main.setup(*this->owner, "Main Color");
         idb.main_oit.setup(*this->owner, "OIT Color");
         idb.sun_shadows.setup(*this->owner, "Sun Shadows");
         {
            auto& list = idb.shadow_casters;
            for (size_t i = 0; i < list.size(); ++i) {
               std::string debug_name;
               #if _DEBUG
                  debug_name = "Shadow Caster ";
                  if (i < 10)
                     debug_name += char('0' + i);
                  else
                     debug_name += "?";
               #endif
               list[i].setup(*this->owner, debug_name);
            }
         }
      }
      //
      // Frustums:
      //
      {
         constexpr VkDeviceSize buffer_size = sizeof(glm::vec4) * 4;
         auto& sf = this->shader_frustums;
         //
         sf.main = this->owner->create_buffer(buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
         sf.sun  = this->owner->create_buffer(buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
         this->owner->set_debug_object_name(sf.main.handle, QString("Buffer: FIF %1 Frustum Buffer (Main)").arg(this->my_index).toStdString());
         this->owner->set_debug_object_name(sf.sun.handle, QString("Buffer: FIF %1 Frustum Buffer (Sun Shadows)").arg(this->my_index).toStdString());
      }
   }
   void frame_in_flight::_setup_command_buffers() {
      this->graphics_commands.setup(*this->owner);
      #if _DEBUG
         this->owner->set_debug_object_name(this->graphics_commands.main_shadow.handle, QString("Frame-in-Flight %1: Command Buffer: Sun Shadows").arg(this->my_index).toStdString());
         this->owner->set_debug_object_name(this->graphics_commands.main.handle,        QString("Frame-in-Flight %1: Command Buffer: Main").arg(this->my_index).toStdString());
         this->owner->set_debug_object_name(this->graphics_commands.fps.handle,         QString("Frame-in-Flight %1: Command Buffer: FPS/UI").arg(this->my_index).toStdString());
      #endif
      {  // Compute
         auto& cc = this->compute_commands;
         cc.frustum_cull_main  = command_buffer(*this->owner);
         cc.frustum_cull_sun   = command_buffer(*this->owner);
         cc.shadow_caster_cull = command_buffer(*this->owner);
         #if _DEBUG
            this->owner->set_debug_object_name(cc.frustum_cull_main.handle,  QString("Frame-in-Flight %1: Command Buffer: Frustum Cull: Main").arg(this->my_index).toStdString());
            this->owner->set_debug_object_name(cc.frustum_cull_sun.handle,   QString("Frame-in-Flight %1: Command Buffer: Frustum Cull: Sun").arg(this->my_index).toStdString());
            this->owner->set_debug_object_name(cc.shadow_caster_cull.handle, QString("Frame-in-Flight %1: Command Buffer: Shadow Caster Cull").arg(this->my_index).toStdString());
         #endif
      }
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
      this->descriptor_sets.free_all(*this->owner);
   }

   void frame_in_flight::teardown() {
      if (!this->owner)
         return;
      this->overlays.fps.teardown_atlas();
      this->invalidate_all_command_buffers();
      this->indirect_draw_commands = {};
      this->idb_mesh_index_staging = {};
      this->idb_params_staging     = {};
      this->shader_frustums = {};
      this->shader_params   = {};
      this->overlays = {};
      //
      this->descriptor_sets.free_all(*this->owner);
      this->graphics_commands = {};
      this->compute_commands  = {};
      //
      // Other resources have their own destructors, but we need to release these explicitly:
      //
      auto* ld = this->owner->logical_device;
      {
         auto destroy_fence = [ld](VkFence& f) {
            vkDestroyFence(ld, f, nullptr);
            f = VK_NULL_HANDLE;
         };
         auto destroy_semaphore = [ld](VkSemaphore& s) {
            vkDestroySemaphore(ld, s, nullptr);
            s = VK_NULL_HANDLE;
         };
         destroy_semaphore(this->semaphores.image_available);
         destroy_semaphore(this->semaphores.compute_finished);
         destroy_semaphore(this->semaphores.graphics_finished);
         destroy_semaphore(this->semaphores.render_finished);
         destroy_fence(this->fences.compute);
         destroy_fence(this->fences.graphics);
      }
      //
      // Done. Guard against accidental redundant teardown calls:
      //
      this->owner = nullptr;
      this->my_index = -1;
   }

   void frame_in_flight::prepare_for_render() {
      this->_update_shader_global_scene_state();
      this->_update_scene_frame_items<rendered_bounds>();
      this->_update_scene_frame_items<rendered_landscape>();
      this->_update_scene_frame_items<rendered_light>();
      this->_update_scene_frame_items<rendered_mesh>();
      this->_update_shader_texture_descriptors(); // can invalidate command buffers, so must run before we check whether command buffers need refilling
      this->owner->scene.update_light_shadows(*this);
      {
         auto& buffer = this->shader_params.active_caster_positions;
         auto& scene  = this->get_scene();
         auto* data   = (glm::vec4*)buffer.map_memory();
         for (size_t i = 0; i < config::max_active_shadow_casters; ++i) {
            size_t light_index = scene.global_state.shadow_caster_index[i];
            if (light_index == std::string::npos) {
               data[i] = glm::vec4(0, 0, 0, 0);
               continue;
            }
            auto& light = scene.entities_of_type<rendered_light>()[light_index];
            data[i]   = light.transform()[3];
            data[i].w = light.frame_drawing_data.radius;
         }
         buffer.flush_memory();
         buffer.unmap_memory(data);
      }
      this->overlays.world_axes.prepare_for_render();
      {
         auto& fps = this->overlays.fps;
         fps.set_value(this->owner->state.fps.display_value());
         if (fps.needs_atlas_update()) {
            fps.generate_atlas(*this->owner, *this);
            this->state.must_re_record_ui = true;
         }
         if (fps.needs_geometry_update()) {
            fps.update_geometry(*this->owner);
         }
         if (this->overlays.world_axes.needs_redraw()) {
            this->state.must_re_record_ui = true;
         }
      }
   }
   namespace {
      template<typename Pred> requires requires(const rendered_mesh& ro, Pred&& predicate) {
         { predicate(ro) } -> std::same_as<bool>;
      }
      void _generate_indirect_draws(
         const scene& scene,
         buffer& staging_params,
         buffer& staging_indices,
         frame_in_flight::indirect_draw_buffers& idb,
         Pred&& predicate
      ) {
         auto& all_meshes = scene.entities_of_type<rendered_mesh>();

         auto* params  = (VkDrawIndexedIndirectCommand*)staging_params.map_memory();
         auto& indices = *idb.mesh_indices.host;
         size_t i = 0; // index into scene mesh array
         size_t j = 0; // index into indirect draw parameter arrays
         for (; i < all_meshes.size(); ++i) {
            auto& ro = all_meshes[i];
            if (!ro.active())
               continue;
            if (!predicate(ro))
               continue;
            params[j] = VkDrawIndexedIndirectCommand{
               .indexCount    = ro.vib().index_count,
               .instanceCount = 1,
               .firstIndex    = 0,
               .vertexOffset  = 0,
               .firstInstance = 0,
            };
            indices[j] = i;
            ++j;
         }
         //
         if (size_t remaining = (scene_entities::max_count_for_type<rendered_mesh> - j)) {
            memset(&params[j], 0, sizeof(VkDrawIndexedIndirectCommand) * remaining);
            //
            // CPU iteration on the index list stops at the first -1.
            //
            indices[j] = -1;
            //
            // The compute shader, however, needs the entire remainder of the list to be 
            // properly updated.
            //
            auto* data = (int32_t*)staging_indices.map_memory();
            memcpy(data, indices.data(), sizeof(frame_in_flight::indirect_draw_buffers::mesh_index_list::value_type) * j);
            cobb::memset(&data[j], int32_t(-1), remaining);
            staging_indices.unmap_memory(data);
         } else {
            auto* data = (int32_t*)staging_indices.map_memory();
            memcpy(data, indices.data(), frame_in_flight::indirect_draw_buffers::mesh_index_list_size);
            staging_indices.unmap_memory(data);
         }
         staging_params.unmap_memory(params);
         //
         staging_params.flush_memory();
         staging_indices.flush_memory();
         idb.params.copy_from(staging_params);
         idb.mesh_indices.gpu.copy_from(staging_indices);
      }
   }
   void frame_in_flight::prepare_indirect_draws() {
      if (!this->state.scene_entity_draws_changed.value_for<rendered_mesh>()) {
         return;
      }
      //
      auto& scene        = this->owner->scene;
      bool  can_do_alpha = this->owner->can_do_alpha();
      //
      _generate_indirect_draws( // sun shadows
         scene,
         this->idb_params_staging,
         this->idb_mesh_index_staging,
         this->indirect_draw_commands.sun_shadows,
         [](const rendered_mesh& ro) {
            return (ro.mesh_flags & rendered_mesh::mesh_flag::cast_shadows) != 0;
         }
      );
      for (auto& caster : this->indirect_draw_commands.shadow_casters) {
         _generate_indirect_draws( // shadow caster
            scene,
            this->idb_params_staging,
            this->idb_mesh_index_staging,
            caster,
            [](const rendered_mesh& ro) {
               return (ro.mesh_flags & rendered_mesh::mesh_flag::cast_shadows) != 0;
            }
         );
      }
      if (can_do_alpha) {
         _generate_indirect_draws( // main
            scene,
            this->idb_params_staging,
            this->idb_mesh_index_staging,
            this->indirect_draw_commands.main,
            [](const rendered_mesh& ro) {
               return !(ro.mesh_flags & rendered_mesh::mesh_flag::requires_oit);
            }
         );
         _generate_indirect_draws( // main OIT
            scene,
            this->idb_params_staging,
            this->idb_mesh_index_staging,
            this->indirect_draw_commands.main_oit,
            [](const rendered_mesh& ro) {
               return (ro.mesh_flags & rendered_mesh::mesh_flag::requires_oit) != 0;
            }
         );
      } else {
         _generate_indirect_draws( // main
            scene,
            this->idb_params_staging,
            this->idb_mesh_index_staging,
            this->indirect_draw_commands.main,
            [](const rendered_mesh& ro) {
               return true;
            }
         );
      }
   }
   void frame_in_flight::record_compute_cull_commands() {
      if (this->state.recorded_compute_cull_commands) {
         return;
      }
      this->state.recorded_compute_cull_commands = true;
      //
      const auto* frustum_cull_shader = this->owner->get_compute_shader(surface_renderer::frustum_cull_shader_id);
      assert(frustum_cull_shader);
      #pragma region Frustum culling: main
      {  // Main
         auto& command_buffer = this->compute_commands.frustum_cull_main;
         auto  command_handle = command_buffer.handle;
         auto& indirect_info  = this->indirect_draw_commands.main;
         //
         command_buffer.reset(0);
         if (auto result = command_buffer.top_level_begin(0); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::record_compute_cull_commands] Failed to begin recording command buffer (main).");
         }
         indirect_info.transfer_queue_ownership_to_compute(command_handle);
         command_buffer.bind_compute_shader_and_descriptors(*frustum_cull_shader, 0, std::array{ this->descriptor_sets.sharing_sets.compute_frustum_culling_main });
         vkCmdDispatch(
            command_handle,
            config::max_rendered_meshes / frustum_cull_shader->metadata.local_size.x,
            1,
            1
         );
         indirect_info.transfer_queue_ownership_to_graphics(command_handle);
         if (auto result = command_buffer.finish(); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::record_compute_cull_commands] Failed to record a command buffer (main).");
         }
      }
      #pragma endregion
      #pragma region Frustum culling: sun shadows
      {  // Sun
         auto& command_buffer = this->compute_commands.frustum_cull_sun;
         auto  command_handle = command_buffer.handle;
         auto& indirect_info  = this->indirect_draw_commands.sun_shadows;
         //
         command_buffer.reset(0);
         if (auto result = command_buffer.top_level_begin(0); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::record_compute_cull_commands] Failed to begin recording command buffer (sun).");
         }
         indirect_info.transfer_queue_ownership_to_compute(command_handle);
         command_buffer.bind_compute_shader_and_descriptors(*frustum_cull_shader, 0, std::array{ this->descriptor_sets.sharing_sets.compute_frustum_culling_sun });
         vkCmdDispatch(
            command_handle,
            config::max_rendered_meshes / frustum_cull_shader->metadata.local_size.x,
            1,
            1
         );
         indirect_info.transfer_queue_ownership_to_graphics(command_handle);
         if (auto result = command_buffer.finish(); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::record_compute_cull_commands] Failed to record a command buffer (sun).");
         }
      }
      #pragma endregion
      #pragma region Radius culling: shadow casters
      {
         auto& command_buffer = this->compute_commands.shadow_caster_cull;
         auto  command_handle = command_buffer.handle;
         //
         command_buffer.reset(0);
         if (auto result = command_buffer.top_level_begin(0); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::record_compute_cull_commands] Failed to begin recording command buffer (main).");
         }

         auto& scene = this->owner->scene;
         for (size_t i = 0; i < config::max_active_shadow_casters; ++i) {
            auto& indirect_info = this->indirect_draw_commands.shadow_casters[i];
            if (scene.global_state.shadow_caster_index[i] < 0) {
               //
               // This transfer may seem redundant, but the graphics-queue commands acquire ownership and then 
               // release it; Vulkan will expect something else to take that ownership and then release it back 
               // to graphics.
               //
               indirect_info.transfer_queue_ownership_to_compute(command_handle);
               indirect_info.transfer_queue_ownership_to_graphics(command_handle);
               continue;
            }
            //
            const compute_shader* shader;
            {
               auto id = surface_renderer::shadow_caster_cull_shader_base_id;
               id.bytes[7] += i;
               shader = this->owner->get_compute_shader(id);
               assert(shader);
            }
            command_buffer.bind_compute_shader_and_descriptors(*shader, 0, std::array{ this->descriptor_sets.sharing_sets.compute_shadow_caster_culls[i] });
            indirect_info.transfer_queue_ownership_to_compute(command_handle);
            vkCmdDispatch(
               command_handle,
               config::max_rendered_meshes / shader->metadata.local_size.x,
               1,
               1
            );
            indirect_info.transfer_queue_ownership_to_graphics(command_handle);
         }

         if (auto result = command_buffer.finish(); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::record_compute_cull_commands] Failed to record a command buffer (main).");
         }
      }
      #pragma endregion
      //
      // Done.
      //
   }
   void frame_in_flight::record_graphics_commands() {
      if (
         this->state.scene_entity_draws_changed.value_for<rendered_mesh>() ||
         this->state.scene_entity_draws_changed.value_for<rendered_landscape>() ||
         this->state.must_re_record_graphics
      ) {
         this->state.must_re_record_graphics = false;
         this->state.scene_entity_draws_changed.for_each([](bool& item) {
            item = false;
         });
         this->_record_scene_draw_commands();
      }
      //
      // TODO: investigate a separate command buffer for landscapes
      //
      if (auto& flag = this->state.scene_entity_draws_changed.value_for<rendered_bounds>()) {
         flag = false;
         this->_record_bounds_draw_commands();
      }
      if (auto& must = this->state.must_re_record_ui) {
         must = false;
         this->_record_ui_draw_commands();
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

      template<typename Prior> requires requires(const rendered_mesh& ro, Prior&& before_draw) {
         { before_draw(ro) };
      }
      void _record_indirect_draws(
         scene& scene,
         command_buffer& command_buffer,
         frame_in_flight::indirect_draw_buffers& indirect_buffer,
         const graphics_shader& shader,
         Prior&& before_draw,
         VkShaderStageFlags push_constant_stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
      ) {
         auto   command_handle = command_buffer.handle;
         size_t first_double_sided_draw = std::string::npos;
         //
         const auto& mesh_indices = *indirect_buffer.mesh_indices.host;
         for (size_t i = 0; i < scene_entities::max_count_for_type<rendered_mesh>; ++i) {
            auto mi = mesh_indices[i];
            if (mi < 0)
               break;
            auto& ro = scene.entities_of_type<rendered_mesh>()[mi];
            if (ro.mesh_flags & rendered_mesh::mesh_flag::double_sided) {
               first_double_sided_draw = i;
               continue;
            }
            before_draw(ro);
            //
            command_buffer.set_pipeline_push_constant(shader.pipeline.layout, push_constant_stages, _make_push_constant_for(ro, mi));
            {
               VkDeviceSize offset = 0;
               auto& vib = ro.vib();
               vkCmdBindVertexBuffers(command_handle, 0, 1, &vib.buffer.handle, &offset);
               if (vib.wide_indices) {
                  vkCmdBindIndexBuffer(command_handle, vib.buffer.handle, vib.indices_at, VK_INDEX_TYPE_UINT32);
               } else {
                  vkCmdBindIndexBuffer(command_handle, vib.buffer.handle, vib.indices_at, VK_INDEX_TYPE_UINT16);
               }
            }
            vkCmdDrawIndexedIndirect(
               command_handle,
               indirect_buffer.params.handle,
               sizeof(VkDrawIndexedIndirectCommand) * i,
               1,
               sizeof(VkDrawIndexedIndirectCommand)
            );
         }
         if (first_double_sided_draw != std::string::npos) {
            auto* variant = shader.get_variant({
               .face_cull_mode = VK_CULL_MODE_NONE,
            });
            assert(variant);
            vkCmdBindPipeline(command_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, variant->handle);
            //
            for (size_t i = first_double_sided_draw; i < config::max_rendered_meshes; ++i) {
               auto mi = mesh_indices[i];
               if (mi < 0)
                  break;
               auto& ro = scene.entities_of_type<rendered_mesh>()[mi];
               if (!(ro.mesh_flags & rendered_mesh::mesh_flag::double_sided)) {
                  continue;
               }
               before_draw(ro);
               //
               command_buffer.set_pipeline_push_constant(shader.pipeline.layout, push_constant_stages, _make_push_constant_for(ro, mi));
               {
                  VkDeviceSize offset = 0;
                  auto& vib = ro.vib();
                  vkCmdBindVertexBuffers(command_handle, 0, 1, &vib.buffer.handle, &offset);
                  if (vib.wide_indices) {
                     vkCmdBindIndexBuffer(command_handle, vib.buffer.handle, vib.indices_at, VK_INDEX_TYPE_UINT32);
                  } else {
                     vkCmdBindIndexBuffer(command_handle, vib.buffer.handle, vib.indices_at, VK_INDEX_TYPE_UINT16);
                  }
               }
               vkCmdDrawIndexedIndirect(
                  command_handle,
                  indirect_buffer.params.handle,
                  sizeof(VkDrawIndexedIndirectCommand) * i,
                  1,
                  sizeof(VkDrawIndexedIndirectCommand)
               );
            }
         }
         //
         // Done.
         //
      }

      void _record_landscape_draws(scene& scene, command_buffer& command_buffer) {
         auto command_handle = command_buffer.handle;
         //
         VkDeviceSize offset = scene.landscape_buffer_vertex_index(0);
         vkCmdBindVertexBuffers(command_handle, 0, 1, &scene.coalesced.landscape_buffer.handle, &offset);
         vkCmdBindIndexBuffer(command_handle, scene.coalesced.landscape_buffer.handle, 0, VK_INDEX_TYPE_UINT16);
         //
         auto& list = scene.entities_of_type<rendered_landscape>();
         for (size_t i = 0; i < list.size(); ++i) {
            auto& item = list[i];
            if (!item.active())
               continue;
            for (size_t j = 0; j < 4; ++j) {
               auto instance_index = i * 4 + j;
               vkCmdDrawIndexed(
                  command_handle,
                  (uint32_t)rendered_landscape::indices_per_quad,
                  1,
                  0,
                  (i * rendered_landscape::vertices_per_mesh) + (j * rendered_landscape::vertices_per_quad),
                  i * 4 + j
               );
            }
         }
      }
   }
   void frame_in_flight::_record_scene_draw_commands() {
      auto& sr    = *this->owner;
      auto& ds    = this->descriptor_sets;
      auto& scene = sr.scene;
      {  // Rendered mesh: shadows, sun
         auto& indirect_info  = this->indirect_draw_commands.sun_shadows;
         auto& command_buffer = this->graphics_commands.main_shadow;
         command_buffer.reset(0);
         if (auto result = command_buffer.top_level_begin(0); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::_refill_command_buffers] Failed to begin recording command buffer (sun shadows).");
         }
         //
         indirect_info.transfer_queue_ownership_to_graphics(command_buffer.handle);
         command_buffer.begin_render_pass(
            *sr.render_passes_by_name.main_shadow,
            sr.canvas.sun_shadow.framebuffer,
            {
               .offset = { 0, 0 },
               .extent = { config::sun_shadow_map_resolution_x, config::sun_shadow_map_resolution_y },
            },
            std::array{
               VkClearValue{ .depthStencil = { config::sun_shadow_invert_depth ? 0.0 : 1.0, 0 } },
            },
            VK_SUBPASS_CONTENTS_INLINE
         );
         {
            const auto* shader = sr.get_graphics_shader(surface_renderer::shader_id_mesh_shadows_sun);
            command_buffer.bind_graphics_shader_and_descriptors(
               *shader,
               0,
               std::array{ ds.scene_state, ds.all_meshes, ds.all_textures }
            );
            _record_indirect_draws(
               scene, command_buffer, indirect_info, *shader,
               [](const rendered_mesh& ro) {}
            );
         }
         {  // Landscape
            command_buffer.bind_graphics_shader_and_descriptors(
               *sr.get_graphics_shader(surface_renderer::shader_id_landscape_shadows_sun),
               0,
               std::array{ ds.scene_state, ds.all_landscapes }
            );
            _record_landscape_draws(scene, command_buffer);
         }
         command_buffer.end_render_pass();
         indirect_info.transfer_queue_ownership_to_compute(command_buffer.handle);
         //
         if (auto result = command_buffer.finish(); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::_record_scene_draw_commands] Failed to record indirect draws for sun shadows.");
         }
      }
      {  // Point light shadows
         auto& command_buffer = this->graphics_commands.main_shadow_placed;
         command_buffer.reset(0);
         if (auto result = command_buffer.top_level_begin(0); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::_refill_command_buffers] Failed to begin recording command buffer (light shadows).");
         }
         //
         for (auto& indirect_info : this->indirect_draw_commands.shadow_casters) {
            indirect_info.transfer_queue_ownership_to_graphics(command_buffer.handle);
         }
         //
         command_buffer.begin_render_pass(
            *sr.render_passes_by_name.main_shadow_placed,
            sr.canvas.light_shadows.framebuffer,
            {
               .offset = { 0, 0 },
               .extent = { config::light_shadow_map_resolution_x, config::light_shadow_map_resolution_y },
            },
            std::array{
               VkClearValue{ .depthStencil = { config::light_shadow_invert_depth ? 0.0 : 1.0, 0 } },
               VkClearValue{ .depthStencil = { config::light_shadow_invert_depth ? 0.0 : 1.0, 0 } },
               VkClearValue{ .depthStencil = { config::light_shadow_invert_depth ? 0.0 : 1.0, 0 } },
               VkClearValue{ .depthStencil = { config::light_shadow_invert_depth ? 0.0 : 1.0, 0 } },
            },
            VK_SUBPASS_CONTENTS_INLINE
         );
         //
         for (size_t i = 0; i < surface_renderer::shadow_caster_count; ++i) {
            {  // Meshes
               auto id = surface_renderer::light_shadow_map_shader_base_id;
               id.bytes[7] += i;
               //
               const auto* shader = sr.get_graphics_shader(id);
               command_buffer.bind_graphics_shader_and_descriptors(
                  *shader,
                  0,
                  std::array{ ds.scene_state, ds.all_textures, ds.all_meshes, ds.all_lights, ds.shadow_caster_map_render }
               );
               _record_indirect_draws(
                  scene, command_buffer, this->indirect_draw_commands.shadow_casters[i], *shader,
                  [](const rendered_mesh& ro) {}
               );
            }
            {  // Landscape
               auto id = surface_renderer::shader_id_landscape_shadows_caster;
               id.bytes[7] += i;
               //
               command_buffer.bind_graphics_shader_and_descriptors(
                  *sr.get_graphics_shader(id),
                  0,
                  std::array{ ds.scene_state, ds.all_landscapes, ds.all_lights, ds.shadow_caster_map_render }
               );
               _record_landscape_draws(scene, command_buffer);
            }
            //
            if (i != surface_renderer::shadow_caster_count - 1)
               command_buffer.next_render_subpass();
         }
         command_buffer.end_render_pass();
         //
         for (auto& indirect_info : this->indirect_draw_commands.shadow_casters) {
            indirect_info.transfer_queue_ownership_to_compute(command_buffer.handle);
         }
         //
         if (auto result = command_buffer.finish(); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::_refill_command_buffers] Failed to record a command buffer (light shadows).");
         }
      }
      {  // Scene objects and landscapes
         auto& indirect_info  = this->indirect_draw_commands.main;
         auto& command_buffer = this->graphics_commands.main;
         auto  command_handle = command_buffer.handle;
         //
         command_buffer.reset(0);
         if (auto result = command_buffer.top_level_begin(0); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::frame_in_flight::_refill_command_buffers] Failed to begin recording command buffer.");
         }
         sr.canvas.color.transition_layout(command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
         //
         indirect_info.transfer_queue_ownership_to_graphics(command_handle);
         //
         command_buffer.begin_render_pass(
            *sr.render_passes_by_name.main,
            sr.canvas.main_framebuffer,
            {
               .offset = { 0, 0 },
               .extent = sr.surface_extent,
            },
            std::array{
               VkClearValue{ .color = {
                  scene.global_state.fog_color_far[0],
                  scene.global_state.fog_color_far[1],
                  scene.global_state.fog_color_far[2],
                  1
               } },
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
         {  // Meshes
            const auto* shader = sr.get_graphics_shader(surface_renderer::main_shader_id);
            command_buffer.bind_graphics_shader_and_descriptors(
               *shader,
               0,
               std::array{
                  ds.scene_state,
                  ds.all_textures,
                  ds.all_meshes,
                  ds.all_lights,
                  ds.shadow_maps,
               }
            );
            //
            if (sr.debug.show_shadow_caster_culling != std::string::npos) {
               auto& shadow_idb = this->indirect_draw_commands.shadow_casters[sr.debug.show_shadow_caster_culling];
               _record_indirect_draws(
                  scene, command_buffer, shadow_idb, *shader,
                  [&set_decal_config](const rendered_mesh& ro) {
                     set_decal_config(ro);
                  }
               );
            } else {
               _record_indirect_draws(
                  scene, command_buffer, this->indirect_draw_commands.main, *shader,
                  [&set_decal_config](const rendered_mesh& ro) {
                     set_decal_config(ro);
                  }
               );
            }
         }
         {  // Landscape
            const auto* shader = sr.get_graphics_shader(surface_renderer::landscape_shader_id);
            command_buffer.bind_graphics_shader_and_descriptors(
               *shader,
               0,
               std::array{
                  ds.scene_state,
                  ds.all_textures,
                  ds.all_landscapes,
                  ds.all_lights,
                  ds.shadow_maps,
               }
            );
            _record_landscape_draws(scene, command_buffer);
            //
            // Debugging:
            //
            if (sr.debug.draw_landscape_wireframe || sr.debug.draw_landscape_normals) {
               const auto* shader = sr.get_graphics_shader(surface_renderer::landscape_wireframe_shader_id);
               //
               // As of this writing, both of these shaders have identical pipeline layouts and use identical 
               // descriptor sets and descriptor set layouts, so we can just bind the descriptor sets once 
               // instead of having to rebind with each shader.
               //
               command_buffer.bind_descriptor_sets(VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.layout, 0, std::array{ ds.scene_state, ds.all_landscapes });
               //
               if (sr.debug.draw_landscape_wireframe) {
                  vkCmdBindPipeline(command_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.handle);
                  _record_landscape_draws(scene, command_buffer);
               }
               if (sr.debug.draw_landscape_normals) {
                  //
                  // If the current hardware doesn't support geometry shaders, then this shader won't be 
                  // loaded. We need to check for a null pointer before trying to use it.
                  //
                  if (const auto* ln_shader = sr.get_graphics_shader(surface_renderer::landscape_normals_shader_id)) {
                     vkCmdBindPipeline(command_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, ln_shader->pipeline.handle);
                     _record_landscape_draws(scene, command_buffer);
                  }
               }
            }
         }
         command_buffer.end_render_pass();
         //
         if (sr.can_do_alpha()) {
            assert(sr.render_passes_by_name.main_oit);
            //
            // OIT passes:
            //
            command_buffer.begin_render_pass(
               *sr.render_passes_by_name.main_oit,
               sr.canvas.oit.framebuffer,
               {
                  .offset = { 0, 0 },
                  .extent = sr.surface_extent,
               },
               std::array{
                  VkClearValue{ .color = { 0, 0, 0, 0 } }, // accumulator
                  VkClearValue{ .color = { 1, 0, 0, 0 } }, // reveal
               },
               VK_SUBPASS_CONTENTS_INLINE
            );
            vkCmdSetDepthBias(command_handle, 0.0, 0.0, 0.0); // binding the landscape shader wiped this state; need to reinitialize it
            last_was_decal = false;
            {
               const auto* shader = sr.get_graphics_shader(surface_renderer::main_shader_oit_color_id);
               command_buffer.bind_graphics_shader_and_descriptors(
                  *shader,
                  0,
                  std::array{
                     ds.scene_state,
                     ds.all_textures,
                     ds.all_meshes,
                     ds.all_lights,
                     ds.shadow_maps,
                  }
               );
               //
               _record_indirect_draws(
                  scene, command_buffer, this->indirect_draw_commands.main_oit, *shader,
                  [&set_decal_config](const rendered_mesh& ro) {
                     set_decal_config(ro);
                  }
               );
            }
            command_buffer.next_render_subpass();
            //
            // Compositing:
            //
            {
               command_buffer.bind_graphics_shader_and_descriptors(
                  *sr.get_graphics_shader(surface_renderer::oit_composite_shader_id),
                  0,
                  std::array{ ds.oit_compositing }
               );
               vkCmdDraw(command_handle, 3, 1, 0, 0);
            }
            command_buffer.end_render_pass();
         }
         indirect_info.transfer_queue_ownership_to_compute(command_handle);
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
      // Done with core commands.
      //
      this->_record_bounds_draw_commands();
   }
   void frame_in_flight::_record_bounds_draw_commands() {
      auto& sr    = *this->owner;
      auto& ds    = this->descriptor_sets;
      auto& scene = sr.scene;
      //
      auto& all_entities = scene.entities_of_type<rendered_bounds>();
      //
      auto& command_buffer = this->graphics_commands.bounds;
      auto  command_handle = command_buffer.handle;
      //
      command_buffer.reset(0);
      if (auto result = command_buffer.top_level_begin(0); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::frame_in_flight::_record_bounds_draw_commands] Failed to begin recording command buffer.");
      }
      command_buffer.begin_render_pass(
         *sr.render_passes_by_name.bounds,
         sr.canvas.main_framebuffer,
         {
            .offset = { 0, 0 },
            .extent = sr.surface_extent,
         },
         std::array<VkClearValue, 0>{},
         VK_SUBPASS_CONTENTS_INLINE
      );
      size_t count = all_entities.size();
      {
         constexpr size_t axis_count     = 3;
         constexpr size_t lines_per_axis = 4;
         constexpr size_t verts_per_line = 2;
         constexpr size_t total_verts    = axis_count * lines_per_axis * verts_per_line;
         //
         const auto* shader = sr.get_graphics_shader(surface_renderer::bounding_box_shader_id);
         command_buffer.bind_graphics_shader_and_descriptors(
            *shader,
            0,
            std::array{ ds.scene_state, ds.all_bounds }
         );
         //
         size_t first = std::string::npos;
         for (size_t i = 0; i < count; ++i) {
            auto& item = all_entities[i];
            if (first == std::string::npos) {
               if (item.active()) {
                  first = i;
                  continue;
               }
            } else {
               if (!item.active()) {
                  vkCmdDraw(command_handle, total_verts, i - first, 0, first);
                  first = std::string::npos;
               }
            }
         }
         if (first != std::string::npos) {
            vkCmdDraw(command_handle, total_verts, count - first, 0, first);
         }
      }
      {
         constexpr size_t axis_count     = 3;
         constexpr size_t verts_per_line = 2;
         constexpr size_t total_verts    = axis_count *  verts_per_line;
         //
         const auto* shader = sr.get_graphics_shader(surface_renderer::bounding_origin_shader_id);
         command_buffer.bind_graphics_shader_and_descriptors(
            *shader,
            0,
            std::array{ ds.scene_state, ds.all_bounds }
         );
         //
         size_t first = std::string::npos;
         for (size_t i = 0; i < count; ++i) {
            auto& item = all_entities[i];
            if (first == std::string::npos) {
               if (item.active()) {
                  first = i;
                  continue;
               }
            } else {
               if (!item.active()) {
                  vkCmdDraw(command_handle, total_verts, i - first, 0, first);
                  first = std::string::npos;
               }
            }
         }
         if (first != std::string::npos) {
            vkCmdDraw(command_handle, total_verts, count - first, 0, first);
         }
      }
      command_buffer.end_render_pass();
      //
      // Done!
      //
      if (auto result = command_buffer.finish(); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::frame_in_flight::_record_bounds_draw_commands] Failed to record a command buffer.");
      }
   }
   void frame_in_flight::_record_ui_draw_commands() {
      auto& sr = *this->owner;
      //
      auto& command_buffer = this->graphics_commands.fps;
      auto  command_handle = command_buffer.handle;
      //
      command_buffer.reset(0);
      if (auto result = command_buffer.top_level_begin(0); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::frame_in_flight::_record_ui_draw_commands] Failed to start recording.");
      }
      //
      this->overlays.fps.commands_pre_pass(command_handle); // commands that must run before vkCmdBeginRenderPass
      //
      command_buffer.begin_render_pass(
         *sr.render_passes_by_name.ui,
         sr.canvas.main_framebuffer,
         {
            .offset = { 0, 0 },
            .extent = sr.surface_extent,
         },
         std::array{
            VkClearValue{ .color        = { 0, 0, 0, 0 } },
            VkClearValue{ .depthStencil = { 1.0, 0 } }, // don't apply inverted depth here; that's per projection matrix
         },
         VK_SUBPASS_CONTENTS_INLINE
      );
      {
         const auto* shader = sr.get_graphics_shader(vulkanDK::overlays::fps::shader_id);
         if (shader) {
            command_buffer.bind_graphics_shader_and_descriptors(*shader, 0, std::array{ this->descriptor_sets.overlay_fps });
            this->overlays.fps.draw_call(command_handle);
         }
      }
      {
         const auto* shader = sr.get_graphics_shader(vulkanDK::overlays::world_axes::shader_id);
         if (shader) {
            command_buffer.bind_graphics_shader_and_descriptors(*shader, 0, std::array{ this->descriptor_sets.overlay_world_axes });
            this->overlays.world_axes.draw_call(command_handle);
         }
      }
      command_buffer.end_render_pass();
      if (auto result = command_buffer.finish(); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::frame_in_flight::_record_ui_draw_commands] Failed to finish recording.");
      }
   }
   void frame_in_flight::submit_compute_cull_commands() {
      vkWaitForFences(this->owner->logical_device, 1, &this->fences.compute, VK_TRUE, UINT64_MAX);
		vkResetFences  (this->owner->logical_device, 1, &this->fences.compute);
      //
      VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
      auto wait_semaphores   = std::array{ this->semaphores.graphics_finished };
      auto signal_semaphores = std::array{ this->semaphores.compute_finished };
      auto command_handles   = std::array{
         this->compute_commands.frustum_cull_main.handle,
         this->compute_commands.frustum_cull_sun.handle,
         this->compute_commands.shadow_caster_cull.handle,
      };
      auto submit_info = VkSubmitInfo{
         .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
         .waitSemaphoreCount   = (uint32_t)wait_semaphores.size(),
         .pWaitSemaphores      = wait_semaphores.data(),
         .pWaitDstStageMask    = waitStages,
         .commandBufferCount   = (uint32_t)command_handles.size(),
         .pCommandBuffers      = command_handles.data(),
         .signalSemaphoreCount = (uint32_t)signal_semaphores.size(),
         .pSignalSemaphores    = signal_semaphores.data(),
      };
      if (this->owner->debug.freeze_culling_updates) {
         if (this->debug.compute_culling_ran_once) {
            submit_info.commandBufferCount = 0;
            submit_info.pCommandBuffers    = nullptr;
         }
      }
      this->debug.compute_culling_ran_once = true;
      if (auto result = vkQueueSubmit(this->owner->queues.compute.handle, 1, &submit_info, this->fences.compute); result != VK_SUCCESS) {
         throw result_exception(result, "[frame_in_flight::submit_compute_cull_commands] Submission failed.");
      }
   }
   void frame_in_flight::submit_graphics_commands(const std::vector<VkCommandBuffer>& append_command_buffers) {
      std::vector<VkCommandBuffer> cb_handles;
      {
         auto& list = this->graphics_commands.list;
         auto  size = list.size();
         cb_handles.resize(size + append_command_buffers.size());
         //
         size_t i = 0;
         for (; i < size; ++i)
            cb_handles[i] = list[i].handle;
         for (auto& item : append_command_buffers)
            cb_handles[i++] = item;
      }
      auto wait_semaphores   = std::array{ this->semaphores.image_available,   this->semaphores.compute_finished };
      auto signal_semaphores = std::array{ this->semaphores.graphics_finished, this->semaphores.render_finished };
      VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };
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
      vkResetFences(this->owner->logical_device, 1, &this->fences.graphics); // set the fence to unsignalled; vkWaitForFences calls will wait for it to be signalled
      if (auto result = vkQueueSubmit(this->owner->queues.graphics.handle, 1, &submit_info, this->fences.graphics); result != VK_SUCCESS) {
         throw result_exception(result, "[frame_in_flight::submit_graphics_commands] Submission failed.");
      }
   }

   void frame_in_flight::invalidate_all_command_buffers() {
      this->command_buffers_invalid = true;
      this->state.recorded_compute_cull_commands = false;
      this->state.must_re_record_graphics = true;
      this->state.must_re_record_ui = true;
      this->debug.compute_culling_ran_once = false;
   }
   
   scene& frame_in_flight::get_scene() {
      assert(this->owner);
      return this->owner->scene;
   }
   void frame_in_flight::_update_shader_global_scene_state() {
      auto& scene = this->get_scene();
      auto& src   = scene.global_state;
      auto& dst   = this->shader_params.scene_data;
      //
      void* data = dst.map_memory();
      memcpy(data, &src, sizeof(src));
      dst.unmap_memory(data);
      //
      // Frustums:
      //
      if (!this->owner->debug.freeze_culling_updates) {
         {
            auto& buffer  = this->shader_frustums.main;
            auto* frustum = buffer.map_memory();
            auto  normals = extract_frustum_normals(scene.global_state.proj * scene.global_state.view);
            memcpy(frustum, &normals, sizeof(normals));
            buffer.unmap_memory(frustum);
         }
         {
            auto& buffer  = this->shader_frustums.sun;
            auto* frustum = buffer.map_memory();
            auto  normals = extract_frustum_normals(scene.global_state.sun_space);
            memcpy(frustum, &normals, sizeof(normals));
            buffer.unmap_memory(frustum);
         }
      }
   }
   void frame_in_flight::_update_shader_texture_descriptors() {
      auto&    scene = this->get_scene();
      auto&    list  = scene.entities_of_type<loaded_texture>();
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
         if (item.lifetime.sync_state.is_up_to_date(this->my_index))
            continue;
         if (item.empty()) // deleted texture
            continue;
         item.lifetime.sync_state.set_up_to_date(this->my_index); // this is only good for single-threaded; for multi-threaded we're gonna need to do this AFTER the descriptor writes go through
         auto view = item.owned_gpu_resources.current.view;
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
      auto& target_set = this->descriptor_sets.all_textures;
      //
      std::vector<VkWriteDescriptorSet> write_info(writes.size());
      for (size_t i = 0; i < writes.size(); ++i) {
         auto& src  = writes[i];
         auto& info = writes[i].entries;
         //
         write_info[i] = VkWriteDescriptorSet{ // texture array
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = target_set,
            .dstBinding      = 1, // this should match the binding value in the shader
            .dstArrayElement = src.start,
            .descriptorCount = (uint32_t)info.size(),
            .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo      = info.data(),
         };
      }
      vkUpdateDescriptorSets(this->owner->logical_device, (uint32_t)write_info.size(), write_info.data(), 0, nullptr);
      //
      // Updating a descriptor set will invalidate any command buffers using it; they must 
      // be reset and their queue regenerated:
      //
      this->invalidate_all_command_buffers();
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::frame_in_flight::_update_shader_texture_descriptors] Invalidated command buffers.");
      }
   }
}