#pragma once
#include <cassert>
#include "helpers/offset_into.h"
#include "./frame_in_flight.h"
#include "./scene.h"
#include "./scene_entities/base.h"
#include "./scene_entities/life_state.h"
#include "./scene_entities/fif_sync_state.h"
#include "./scene_entities/scene_limits.h"
#include "./scene_entities/concepts/has_frame_culling_data.h"
#include "./scene_entities/concepts/has_frame_drawing_data.h"
#include "./scene_entities/concepts/owns_gpu_resources.h"

namespace vulkanDK {
   template<typename Entity>
   void frame_in_flight::_resize_frame_data_buffers() {
      auto& scene = this->get_scene();

      //
      // Sadly, we can't just copy the old buffers' content into the new buffers to save on a CPU-to-GPU 
      // data transfer, as the old buffers don't have the TRANSFER_SRC usage bit set. We'll just have to 
      // mark all entities as out-of-date.
      //

      if constexpr (scene_entities::concepts::has_frame_culling_data<Entity>) {
         VkDeviceSize buffer_size = scene.max_entity_slots<Entity>() * sizeof(Entity::frame_culling_data_type);
         //
         auto& buffer = this->shader_params.scene_entity_frame_culling_data.value_for<Entity>();
         buffer = this->_create_buffer(buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
         this->_set_debug_object_name(
            buffer,
            QString("Buffer: FIF %1 Frame Culling Data Buffer (%2)")
               .arg(this->my_index)
               .arg(Entity::name_plural)
               .toStdString()
                  .data()
         );
         static_assert(
            !scene_entities::concepts::has_frame_culling_data<Entity>,
            "If we allowed resizing rendered_mesh's buffers, then this here would not be sufficient: the buffers in "
            "`indirect_draw_buffers` would need to be resized as well. Of course, as of this writing, we don't have "
            "any entity types that use both culling data and an adjustable maximum count. If that ever changes, will "
            "they need to be handled the same way rendered_mesh would need to be? Who knows?"
         );
         //
         // Update descriptors:
         //
         static_assert(
            !scene_entities::concepts::has_frame_culling_data<Entity>,
            "You need to implement post-buffer-resize descriptor updates for this entity type's frame culling data."
         );
      }
      if constexpr (scene_entities::concepts::has_frame_drawing_data<Entity>) {
         VkDeviceSize buffer_size = scene.max_entity_slots<Entity>() * sizeof(Entity::frame_drawing_data_type);
         //
         auto& buffer = this->shader_params.scene_entity_frame_drawing_data.value_for<Entity>();
         buffer = this->_create_buffer(buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
         this->_set_debug_object_name(
            buffer,
            QString("Buffer: FIF %1 Frame Drawing Data Buffer (%2)")
               .arg(this->my_index)
               .arg(Entity::name_plural)
               .toStdString()
                  .data()
         );
         //
         // Update descriptors:
         //
         auto buffer_info = VkDescriptorBufferInfo{
            .buffer = buffer.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE,
         };
         auto write_info = VkWriteDescriptorSet{ // storage buffer object
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet           = VK_NULL_HANDLE,
            .dstBinding       = 0,
            .dstArrayElement  = 0,
            .descriptorCount  = 1, // this should be 1 because we are updating 1 buffer; that the buffer's data is used as an array on the shader side is irrelevant
            .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo       = nullptr,
            .pBufferInfo      = &buffer_info,
            .pTexelBufferView = nullptr,
         };
         if constexpr (std::is_same_v<Entity, rendered_bounds>) {
            write_info.dstSet = this->descriptor_sets.all_bounds;
         } else if constexpr (std::is_same_v<Entity, rendered_landscape>) {
            write_info.dstSet = this->descriptor_sets.all_landscapes;
         } else if constexpr (std::is_same_v<Entity, rendered_light>) {
            write_info.dstSet = this->descriptor_sets.all_lights;
         } else if constexpr (std::is_same_v<Entity, rendered_mesh>) {
            write_info.dstSet = this->descriptor_sets.all_meshes;
         }
         assert(write_info.dstSet != VK_NULL_HANDLE);
         vkUpdateDescriptorSets(this->_logical_device_handle(), 1, &write_info, 0, nullptr);
      }

      //
      // Mark all entities as out of date, so that we update the buffers' contents as appropriate.
      //

      for (auto& item : scene.entities_of_type<Entity>()) {
         if (!item.active())
            continue;
         item.lifetime.sync_state.set_out_of_date(this->my_index);
      }
   }

   namespace {
      struct _vib_size_info {
         VkDeviceSize all_data      = 0;
         VkDeviceSize all_indices   = 0;
         VkDeviceSize vb_per_entity = 0;
         VkDeviceSize ib_per_entity = 0;
      };

      template<typename Entity>
      void _get_vib_sizes(size_t max_entity_count, _vib_size_info& si) {
         constexpr const auto& settings = Entity::coalesced_vib_settings;

         si.vb_per_entity = sizeof(Entity::coalesced_vertex_type) * settings.fixed_vertex_count;
         si.ib_per_entity = 0;

         si.all_data    = max_entity_count * si.vb_per_entity;
         si.all_indices = 0;
         {
            si.ib_per_entity = 0;
            if constexpr (settings.fixed_index_count > 0) {
               si.ib_per_entity += sizeof(Entity::coalesced_index_type) * settings.fixed_index_count;
            } else {
               si.ib_per_entity += sizeof(Entity::coalesced_index_type) * settings.fixed_vertex_count;
            }
            if constexpr (!settings.constant_shared_indices) {
               si.all_indices = si.ib_per_entity * max_entity_count;
            } else {
               si.all_indices   = si.ib_per_entity;
               si.ib_per_entity = 0;
            }
            si.all_data += si.all_indices;
         }
      }
   }

   template<typename Entity>
   void frame_in_flight::_resize_scene_entity_coalesced_vib() {
      constexpr const auto& settings = Entity::coalesced_vib_settings;

      auto& scene = this->get_scene();
      if constexpr (scene_entities::all_types_with_fixed_length_coalesced_vibs::contains_type<Entity>) {
         auto& buffer    = this->coalesced_vibs.fixed_length.value_for<Entity>();
         auto  max_count = scene.max_entity_slots<Entity>();

         _vib_size_info sizes;
         _get_vib_sizes<Entity>(max_count, sizes);

         buffer = this->_create_buffer(sizes.all_data, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
         this->_set_debug_object_name(
            buffer,
            QString("Buffer: FIF %1 Coalesced VIB Buffer, Fixed-Length (%1)")
               .arg(this->my_index)
               .arg(Entity::name_plural)
               .toStdString()
                  .data()
         );

         if constexpr (settings.constant_shared_indices) {
            auto  staging = this->_create_staging_buffer(sizes.all_indices);
            void* data    = staging.map_memory();
            Entity::coalesce_constant_shared_indices_into(data);
            staging.unmap_memory(data);
            buffer.copy_from(staging);
         }
      } else {
         static_assert(scene_entities::all_types_with_fixed_length_coalesced_vibs::contains_type<Entity>, "VARIABLE-LENGTH COALESCED VIBS NOT IMPLEMENTED");

         auto& list  = scene.entities_of_type<Entity>();
         auto  count = list.size();
         //
         // TODO: update the `coalesced_vib`?
      }

      this->state.scene_entity_coalesced_vib_resize_needed.value_for<Entity>() = false;

      for (auto& entity : scene.entities_of_type<Entity>()) {
         if (!entity.active())
            continue;
         entity.lifetime.coalescing.sync_state.set_out_of_date(this->my_index);
      }
   }
   template<typename Entity>
   void frame_in_flight::_update_scene_entity_coalesced_vib() {
      constexpr const auto& settings = Entity::coalesced_vib_settings;

      auto&  scene = this->get_scene();
      auto&  list  = scene.entities_of_type<Entity>();
      size_t count = list.size();
      if constexpr (scene_entities::all_types_with_fixed_length_coalesced_vibs::contains_type<Entity>) {
         size_t first_dirty = 0;
         bool   any_dirty   = false;
         for (size_t i = 0; i < count; ++i) {
            auto& item = list[i];
            if (!item.active()) {
               item.lifetime.coalescing.sync_state.set_up_to_date(this->my_index);
               continue;
            }
            if (item.lifetime.coalescing.sync_state.is_up_to_date(this->my_index))
               continue;
            first_dirty = i;
            any_dirty   = true;
            break;
         }
         if (!any_dirty)
            return;

         auto& coalesced_vib = this->coalesced_vibs.fixed_length.value_for<Entity>();
         auto  max_count     = scene.max_entity_slots<Entity>();

         _vib_size_info sizes;
         _get_vib_sizes<Entity>(max_count, sizes);

         //
         // We're only going to copy over everything at and after the first out-of-date entity, 
         // so as to minimize how much work we need to do and how much data we need to send (at 
         // least in the more optimal cases).
         //

         size_t count_to_update = (count - first_dirty);
         buffer staging_i;
         buffer staging_v = this->_create_staging_buffer(sizes.vb_per_entity * count_to_update);
         void*  memory_i;
         void*  memory_v  = staging_v.map_memory();
         if constexpr (!settings.constant_shared_indices) {
            staging_i = this->_create_staging_buffer(sizes.ib_per_entity * count_to_update);
            memory_i  = staging_i.map_memory();
         }
         
         for (size_t i = first_dirty; i < list.size(); ++i) {
            auto& item = list[i];
            if (!item.active()) {
               item.lifetime.coalescing.sync_state.set_up_to_date(this->my_index);
               continue;
            }
            /*//
            //
            // DO NOT skip over items that are up to date. The use of a staging buffer means that 
            // anything we don't copy over will be lost!
            //
            if (item.lifetime.coalescing.sync_state.is_up_to_date(this->my_index))
               continue;
            //*/

            if constexpr (!settings.constant_shared_indices) {
               item.coalesce_indices_into(cobb::offset_into(memory_i, (i - first_dirty) * sizes.ib_per_entity));
            }
            item.coalesce_vertices_into(cobb::offset_into(memory_v, (i - first_dirty) * sizes.vb_per_entity));
            
            item.lifetime.coalescing.sync_state.set_up_to_date(this->my_index);
         }

         if constexpr (!settings.constant_shared_indices) {
            staging_i.unmap_memory(memory_i);
         }
         staging_v.unmap_memory(memory_v);

         if constexpr (!settings.constant_shared_indices) {
            coalesced_vib.copy_from(staging_i, VkBufferCopy{
               .srcOffset = 0,
               .dstOffset = (first_dirty * sizes.ib_per_entity),
               .size = sizes.ib_per_entity * count_to_update,
            });
         }
         coalesced_vib.copy_from(staging_v, VkBufferCopy{
            .srcOffset = 0,
            .dstOffset = sizes.all_indices + (first_dirty * sizes.vb_per_entity),
            .size      = sizes.vb_per_entity * count_to_update,
         });
      } else {
         static_assert(scene_entities::all_types_with_fixed_length_coalesced_vibs::contains_type<Entity>, "VARIABLE-LENGTH COALESCED VIBS NOT IMPLEMENTED");

         auto& list  = scene.entities_of_type<Entity>();
         auto  count = list.size();
         //
         // TODO: update the `coalesced_vib`
      }
   }

   template<typename Entity>
   void frame_in_flight::_update_drawn_scene_entity_frame_data() {
      constexpr bool frame_drawing_data_only = !Entity::is_drawn && scene_entities::concepts::has_frame_drawing_data<Entity> && !scene_entities::concepts::owns_gpu_resources<Entity>;

      auto&  scene = this->get_scene();
      auto&  list  = scene.entities_of_type<Entity>();
      size_t count = list.size();
      {
         constexpr size_t max_count = scene_entities::max_count_for_type<Entity>;
         if constexpr (max_count > 0) {
            assert(count <= max_count);
         }
      }
      //
      // Find the first scene item in need of an update.
      //
      size_t first_dirty = 0;
      bool   any_dirty   = false;
      for (size_t i = 0; i < count; ++i) {
         auto& item       = list[i];
         auto& sync_state = item.lifetime.sync_state;
         switch (item.lifetime.life_state) {
            case scene_entities::life_state::empty:
               continue;
            case scene_entities::life_state::pending_delete:
               //
               // If the entity consists entirely of frame drawing data, then we want to give that data an 
               // opportunity to clear. We need this for things like `rendered_light`, where the content 
               // of its frame drawing data indicates whether it exists (e.g. a light that doesn't exist 
               // has zero brightness).
               //
               if constexpr (!frame_drawing_data_only) {
                  sync_state.set_up_to_date(this->my_index);
                  continue;
               }
               break;
         }
         //
         // Scene object is "active."
         //
         if (sync_state.is_up_to_date(this->my_index))
            continue;
         first_dirty = i;
         any_dirty   = true;
         break;
      }
      //
      if (any_dirty) {
         auto& all_culling_buffers = this->shader_params.scene_entity_frame_culling_data;
         auto& all_drawing_buffers = this->shader_params.scene_entity_frame_drawing_data;

         typename Entity::frame_culling_data_type* cull_buffer = nullptr;
         typename Entity::frame_drawing_data_type* draw_buffer = nullptr;
         //
         // NOTE: VMA always maps entire buffers, so there's no point in trying to 
         //       only map the parts we need to update.
         //
         if constexpr (scene_entities::concepts::has_frame_culling_data<Entity>) {
            cull_buffer = (typename Entity::frame_culling_data_type*) all_culling_buffers.value_for<Entity>().map_memory();
         }
         if constexpr (scene_entities::concepts::has_frame_drawing_data<Entity>) {
            draw_buffer = (typename Entity::frame_drawing_data_type*) all_drawing_buffers.value_for<Entity>().map_memory();
         }

         for (size_t i = first_dirty; i < count; ++i) {
            auto& item       = list[i];
            auto& sync_state = item.lifetime.sync_state;
            if (!item.active()) {
               if constexpr (frame_drawing_data_only) {
                  if (item.pending_delete())
                     sync_state.set_up_to_date(this->my_index);
                  continue;
               } else {
                  //
                  // If the entity consists entirely of frame drawing data, then we want to give that data an 
                  // opportunity to clear.
                  //
                  if (!item.pending_delete()) {
                     continue;
                  }
               }
            }
            if (sync_state.is_up_to_date(this->my_index))
               continue;

            if constexpr (scene_entities::concepts::has_frame_drawing_data<Entity>) {
               auto& src = item.frame_drawing_data;
               auto& dst = draw_buffer[i];
               memcpy(&dst, &src, sizeof(Entity::frame_drawing_data_type));
            }
            if constexpr (scene_entities::concepts::has_frame_culling_data<Entity>) {
               cull_buffer[i] = item.calculate_frame_culling_data();
            }
            
            sync_state.set_up_to_date(this->my_index);
         }

         if constexpr (scene_entities::concepts::has_frame_culling_data<Entity>) {
            auto& buf = all_culling_buffers.value_for<Entity>();
            buf.flush_memory();
            buf.unmap_memory(cull_buffer);
         }
         if constexpr (scene_entities::concepts::has_frame_drawing_data<Entity>) {
            auto& buf = all_drawing_buffers.value_for<Entity>();
            buf.unmap_memory(draw_buffer);
         }
      }
   }
}