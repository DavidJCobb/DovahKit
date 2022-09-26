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
            item.lifetime.life_state = scene_entities::life_state::active_recycle;
            item.lifetime.sync_state.set_all_out_of_date();
            return i;
         }
      }
      if constexpr (scene_entities::max_count_for_type<Entity> > 0) {
         if (size >= scene_entities::max_count_for_type<Entity>)
            return index_of_none;
      } else {
         auto max_count = this->max_entity_slots<Entity>();
         if (max_count > 0 && size >= max_count)
            return index_of_none;
      }
      auto& item = list.emplace_back();
      item.lifetime.life_state = scene_entities::life_state::active;
      item.lifetime.sync_state.set_all_out_of_date();
      return size;
   }

   template<typename Entity> size_t scene::max_entity_slots() const noexcept {
      constexpr const size_t fixed_maximum = scene_entities::max_count_for_type<Entity>;
      if (fixed_maximum)
         return fixed_maximum;
      //
      // This entity count's maximum can vary depending on the scene configuration.
      //
      if constexpr (std::is_same_v<Entity, rendered_landscape>) {
         return this->config.landscape_grid_side_length * this->config.landscape_grid_side_length;
      }
      return 0;
   }
   template<typename Entity> size_t scene::entity_slots_available() const noexcept {
      size_t max_slots = max_entity_slots<Entity>(); // the variable name `slots` cannot safely be used in any Qt-based program
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
