#pragma once
#include "./scene.h"
#include "./scene_entities/concepts/owns_gpu_resources.h"
#include "./scene_entities/base.h"
#include "./scene_entities/scene_limits.h"

namespace vulkanDK {
   template<typename Entity> size_t scene::insert_new_scene_entity() {
      auto& list = this->entities_of_type<Entity>();
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.empty()) {
            item.lifetime.life_state = scene_entities::life_state::active;
            item.lifetime.sync_state.set_all_out_of_date();
            if constexpr (scene_entities::all_types_with_coalesced_vibs::contains_type<Entity>) {
               item.lifetime.coalescing.sync_state.set_all_out_of_date();
            }
            return i;
         }
         if (item.pending_delete()) {
            if constexpr (scene_entities::concepts::owns_gpu_resources<Entity>) {
               //
               // Do not recycle if the entity has both a "current" and an "outdated" owned GPU resource. 
               // This should only occur if a marked-for-delete entity is marked for recycle, and then 
               // marked for deletion again before it can be recycled. Such entities should be considered 
               // irrecoverable; just let them get deleted.
               //
               auto& res = item.owned_gpu_resources;
               if (res.has_current() && res.has_outdated()) {
                  continue;
               }
            }
            item.lifetime.life_state = scene_entities::life_state::active;
            item.lifetime.recycling  = true;
            item.lifetime.sync_state.set_all_out_of_date();
            if constexpr (scene_entities::all_types_with_coalesced_vibs::contains_type<Entity>) {
               item.lifetime.coalescing.sync_state.set_all_out_of_date();
            }
            return i;
         }
      }
      if (size >= scene_entities::max_count_for_type<Entity>)
         return index_of_none;
      auto& item = list.emplace_back();
      item.lifetime.life_state = scene_entities::life_state::active;
      item.lifetime.sync_state.set_all_out_of_date();
      if constexpr (scene_entities::all_types_with_coalesced_vibs::contains_type<Entity>) {
         item.lifetime.coalescing.sync_state.set_all_out_of_date();
      }
      return size;
   }

   template<typename Entity> size_t scene::entity_slots_available() const noexcept {
      size_t max_slots = scene_entities::max_count_for_type<Entity>; // the variable name `slots` cannot safely be used in any Qt-based program
      //
      auto&  list  = this->entities_of_type<Entity>();
      size_t size  = list.size();
      for (auto& item : list) {
         if (!item.empty()) {
            --max_slots;
            if (!max_slots)
               break;
         }
      }
      return max_slots;
   }
}
