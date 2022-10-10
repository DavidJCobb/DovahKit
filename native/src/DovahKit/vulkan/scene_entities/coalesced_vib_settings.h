#pragma once
#include <array>
#include <type_traits>
#include "../_vulkan.h"

namespace vulkanDK::scene_entities {
   template<size_t AdditionalSharedIndexSets = 0>
   struct coalesced_vib_settings {

      // A coalesced vertex-and-index buffer with constant shared indices enabled can 
      // have multiple sets of shared indices at the start of the VIB data, between 
      // the "main" set and the vertex data.
      struct additional_shared_index_set {
         size_t index_count = 0;
      };

      bool enabled = false;
      //
      std::array<additional_shared_index_set, AdditionalSharedIndexSets> additional_shared_index_sets = {};
      bool   constant_shared_indices = false; // true if all entities have the exact same indices and those indices never change
      size_t fixed_vertex_count      = 0;     // vertex count per entity; 0 if the vertex count can vary between different entities
      size_t fixed_index_count       = 0;     // index  count per entity; 0 to use the default (same as vertex count)

      constexpr bool is_fixed_size() const noexcept {
         if (this->fixed_vertex_count == 0)
            return false;
         return true;
      }

      struct size_info {
         struct additional_shared_index_set {
            VkDeviceSize offset = 0;
            VkDeviceSize size   = 0;
         };

         VkDeviceSize total_size = 0;
         //
         VkDeviceSize shared_indices_size = 0;
         std::array<additional_shared_index_set, AdditionalSharedIndexSets> additional_shared_index_sets = {};
         VkDeviceSize vertices_offset = 0;
         //
         VkDeviceSize verts_bytes_per_entity = 0;
         VkDeviceSize index_bytes_per_entity = 0;

         constexpr VkDeviceSize offset_of_indices_of(size_t entity_index) const noexcept {
            return this->shared_indices_size + (this->index_bytes_per_entity * entity_index);
         }
         constexpr VkDeviceSize offset_of_vertices_of(size_t entity_index) const noexcept {
            return this->vertices_offset + (this->verts_bytes_per_entity * entity_index);
         }
      };
   };
}