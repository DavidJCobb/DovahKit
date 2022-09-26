#pragma once
#include <cassert>
#include "frame_in_flight.h"
#include "scene.h"
#include "./scene_entities/base.h"
#include "./scene_entities/life_state.h"
#include "./scene_entities/fif_sync_state.h"
#include "./scene_entities/scene_limits.h"
#include "./scene_entities/concepts/has_frame_culling_data.h"
#include "./scene_entities/concepts/has_frame_drawing_data.h"
#include "./scene_entities/concepts/owns_gpu_resources.h"

namespace vulkanDK {
   template<typename Entity>
   void frame_in_flight::_update_scene_frame_items() {
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
               // opportunity to clear.
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