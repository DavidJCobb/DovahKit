#pragma once
#include <cstdint>
#include <vector>
#include "helpers/widen_u16_to_u32.h"
#include "../_vulkan.h"
#include "../buffer.h"
#include "../helpers/vertex_index_list.h"

namespace vulkanDK::scene_entities {
   template<typename Entity, typename VertexType>
   class coalesced_vib {
      public:
         using entity_type = Entity;
         using vertex_type = VertexType;
         using vertex_index_type = uint32_t;

         using entity_list_type  = std::vector<entity_type>;
         using entity_index_type = size_t;

         static constexpr const size_t index_of_none = -1;

      public:
         struct entry_data {
            size_t   first_user   = index_of_none; // index of first entity to use this entry
            size_t   refcount     = 0;
            uint32_t vertex_count = 0;
         };
      public:
         struct {
            buffer vertices;
            buffer indices;
         } buffers;
         std::vector<entry_data> entries;
         std::vector<size_t> map; // map entity indices to entry indices

         bool prune_needed   = false;
         bool rebuild_needed = false;

         void prune_entries() {
            //
            // Manual erase-remove idiom so we can also update indices in `this->map`: 
            // we "erase" N items starting at position X by overwriting every entry at 
            // index X or above with the entry at index (X + N) or above.
            //
            size_t erased = 0;
            for (size_t i = 0; i < this->entries.size(); ++i) {
               auto& entry = this->entries[i];
               if (erased > 0) {
                  entry = this->entries[i + erased];
               }
               if (entry.refcount)
                  continue;
               //
               ++erased;
               for (auto& j : this->map) {
                  if (j == index_of_none)
                     continue;
                  if (j > i)
                     j -= erased;
               }
            }
            if (erased)
               this->entries.resize(this->entries.size() - erased);
            //
            this->prune_needed = false;
         }

         void calc_buffer_sizes(VkDeviceSize& ib, VkDeviceSize& vb) const noexcept {
            ib = 0;
            vb = 0;

            VkDeviceSize index_count = 0;
            for (auto& entry : this->entries)
               index_count += entry.vertex_count;

            ib = index_count * sizeof(vertex_index_type);
            vb = index_count * sizeof(vertex_type);
         }

         void add_entity(entity_index_type added_index, const entity_list_type& list) {
            if (this->map.size() < added_index + 1) {
               this->map.resize(added_index + 1, index_of_none);
            }
            auto& added = list[added];
            for (size_t i = 0; i < list.size(); ++i) {
               if (i == added_index)
                  continue;
               if (this->map[i] == index_of_none)
                  continue;
               if (added.mesh_data() == list[i].mesh_data()) {
                  this->map[added_index] = this->map[i];
                  ++this->entries[this->map[i]].refcount;
                  return;
               }
            }
            this->rebuild_needed = true;

            auto  entry_index = this->entries.size();
            auto& entry       = this->entries.emplace_back();
            this->map[added_index] = entry_index;
            ++entry.refcount;
            entry.first_user   = added_index;
            entry.vertex_count = added.vertex_indices().size();
         }

         void remove_entity(entity_index_type removed_index) {
            if (removed_index >= this->map.size()) {
               return;
            }
            auto entry_index = this->map[removed_index];
            if (entry_index == index_of_none) {
               return;
            }
            auto& entry = this->entries[entry_index];
            assert(entry.refcount && "The refcount shouldn't already be zero when removing a reference.");
            --entry.refcount;
            if (entry.refcount > 0) {
               if (entry.first_user == removed_index) {
                  entry.first_user = index_of_none;
                  for (size_t i = 0; i < this->map.size(); ++i) {
                     if (this->map[i] == entry_index) {
                        entry.first_user = i;
                        break;
                     }
                  }
                  assert(entry.first_user != index_of_none && "How can an entry have no other users if its refcount is non-zero after removing this one?");
               }
            } else {
               this->prune_needed   = true;
               this->rebuild_needed = true;
            }
         }

         void entity_vib_changed(entity_index_type changed_index, const entity_list_type& list) {
            this->remove_entity(changed_index);
            this->add_entity(changed_index, list);
         }
         
         void update_buffers(std::vector<entity_type>& all_entities) {
            if (this->prune_needed) {
               this->prune_entries();
            }

            auto  staging_i = sr.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            auto  staging_v = sr.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            void* data_i = staging_i.map_memory();
            void* data_v = staging_v.map_memory();

            size_t vertex_index = 0;
            for (auto& entry : this->entries) {
               assert(entry.refcount && "Unused entries should've been removed at this stage.");
               assert(entry.first_user != index_of_none && "We should know by now what entity to pull data from.");
               const entity_type&       entity      = all_entites[entry.first_user];
               const vertex_index_list& src_indices = entity.vertex_indices();
               assert(entry.vertex_count == src_indices.size());
               //
               {
                  auto* dst = data_i + (sizeof(vertex_index_type) * vertex_index);
                  if (src_indices.type() == vertex_index_list::value_type::thin) {
                     cobb::widen_u16_to_u32(src_indices.size(), src_indices.thin_data(), dst);
                  } else {
                     memcpy(dst, src_indices.wide_data(), entry.vertex_count);
                  }
               }
               memcpy(data_v + (sizeof(vertex_type) * vertex_index), entity.vertices().data(), entry.vertex_count);
               //
               vertex_index += entity.vertex_count;
            }

            staging_i.unmap_memory(data_i);
            staging_v.unmap_memory(data_v);
            this->buffers.indices.copy_from(staging_i);
            this->buffers.vertices.copy_from(staging_v);
            
            this->rebuild_needed = false;
         }
   };
}