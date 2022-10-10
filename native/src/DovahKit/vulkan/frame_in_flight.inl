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
#include "./scene_entities/traits/coalesced_vib_fixed_size_info.h"

namespace vulkanDK {
   template<typename Entity>
   void frame_in_flight::_allocate_scene_entity_coalesced_vib() {
      constexpr const auto& settings = Entity::coalesced_vib_settings;

      auto& scene = this->get_scene();
      if constexpr (scene_entities::all_types_with_fixed_length_coalesced_vibs::contains_type<Entity>) {
         auto& buffer    = this->coalesced_vibs.fixed_length.value_for<Entity>();
         auto  max_count = scene_entities::max_count_for_type<Entity>;

         constexpr auto sizes = scene_entities::traits::coalesced_vib_fixed_size_info<Entity>();

         buffer = this->_create_buffer(sizes.total_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
         this->_set_debug_object_name(
            buffer,
            QString("Buffer: FIF %1 Coalesced VIB Buffer, Fixed-Length (%1)")
               .arg(this->my_index)
               .arg(Entity::name_plural)
               .toStdString()
                  .data()
         );

         if constexpr (settings.constant_shared_indices) {
            auto  staging = this->_create_staging_buffer(sizes.shared_indices_size);
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

         constexpr auto sizes = scene_entities::traits::coalesced_vib_fixed_size_info<Entity>();

         //
         // We're only going to copy over everything at and after the first out-of-date entity, 
         // so as to minimize how much work we need to do and how much data we need to send (at 
         // least in the more optimal cases).
         //

         size_t count_to_update = (count - first_dirty);
         buffer staging_i;
         buffer staging_v = this->_create_staging_buffer(sizes.verts_bytes_per_entity * count_to_update);
         void*  memory_i;
         void*  memory_v  = staging_v.map_memory();
         if constexpr (!settings.constant_shared_indices) {
            staging_i = this->_create_staging_buffer(sizes.index_bytes_per_entity * count_to_update);
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
               item.coalesce_indices_into(cobb::offset_into(memory_i, (i - first_dirty) * sizes.index_bytes_per_entity));
            }
            item.coalesce_vertices_into(cobb::offset_into(memory_v, (i - first_dirty) * sizes.verts_bytes_per_entity));
            
            item.lifetime.coalescing.sync_state.set_up_to_date(this->my_index);
         }

         if constexpr (!settings.constant_shared_indices) {
            staging_i.unmap_memory(memory_i);
         }
         staging_v.unmap_memory(memory_v);

         if constexpr (!settings.constant_shared_indices) {
            coalesced_vib.copy_from(staging_i, VkBufferCopy{
               .srcOffset = 0,
               .dstOffset = (first_dirty * sizes.index_bytes_per_entity),
               .size = sizes.index_bytes_per_entity * count_to_update,
            });
         }
         coalesced_vib.copy_from(staging_v, VkBufferCopy{
            .srcOffset = 0,
            .dstOffset = sizes.vertices_offset + (first_dirty * sizes.verts_bytes_per_entity),
            .size      = sizes.verts_bytes_per_entity * count_to_update,
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