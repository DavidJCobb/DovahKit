#pragma once
#include "../coalesced_vib_settings.h"
#include "../scene_limits.h"

namespace vulkanDK::scene_entities::traits {
   template<typename Entity>
   constexpr auto coalesced_vib_fixed_size_info() noexcept {
      constexpr const auto& settings         = Entity::coalesced_vib_settings;
      constexpr const auto  max_entity_count = scene_entities::max_count_for_type<Entity>;
      constexpr const auto  sizeof_vertex = sizeof(typename Entity::coalesced_vertex_type);
      constexpr const auto  sizeof_index  = sizeof(typename Entity::coalesced_index_type);

      typename std::decay_t<decltype(settings)>::size_info si = {};

      si.shared_indices_size    = 0;
      si.verts_bytes_per_entity = sizeof_vertex * settings.fixed_vertex_count;
      si.index_bytes_per_entity = 0;

      si.total_size      = max_entity_count * si.verts_bytes_per_entity;
      si.vertices_offset = 0;
      {
         if constexpr (settings.fixed_index_count > 0) {
            si.index_bytes_per_entity += sizeof_index * settings.fixed_index_count;
         } else {
            si.index_bytes_per_entity += sizeof_index * settings.fixed_vertex_count;
         }
         if constexpr (!settings.constant_shared_indices) {
            si.vertices_offset = si.index_bytes_per_entity * max_entity_count;
            static_assert(settings.additional_shared_index_sets.size() == 0, "This feature is not supported when indices in general are not shared.");
         } else {
            si.vertices_offset = si.index_bytes_per_entity;
            si.index_bytes_per_entity = 0;
            //
            for (size_t i = 0; i < settings.additional_shared_index_sets.size(); ++i) {
               const auto& set = settings.additional_shared_index_sets[i];
               auto& dst = si.additional_shared_index_sets[i];
               //
               size_t index_count = 0;
               if (set.index_count > 0) {
                  index_count = set.index_count;
               } else {
                  if (settings.fixed_index_count > 0) {
                     index_count = settings.fixed_index_count;
                  } else {
                     index_count = settings.fixed_vertex_count;
                  }
               }
               dst.offset = si.vertices_offset;
               dst.size   = sizeof_index * index_count;
               si.vertices_offset += dst.size;
            }
            si.shared_indices_size = si.vertices_offset;
         }
      }
      if (auto misalign = si.vertices_offset % 4; misalign) {
         si.vertices_offset += 4 - misalign;
      }
      si.total_size += si.vertices_offset;

      return si;
   }
}