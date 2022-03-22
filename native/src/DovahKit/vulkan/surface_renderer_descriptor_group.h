#pragma once
#include <array>
#include <type_traits>
#include "_vulkan.h"
#include "descriptor_definitions.h"

namespace vulkanDK {
   class surface_renderer;

   template<typename T> requires (std::is_same_v<T, VkDescriptorSet> || std::is_same_v<T, descriptor_set_layout>)
   struct surface_renderer_dsl_group {
      public:
         union {
            std::array<T, 6> list = {};
            struct {
               T oit_composite;
               T sun_shadows;
               T light_shadows;
               T standard;
               T fps;
               T world_axes;
            };
         };

         static constexpr size_t size = std::tuple_size_v<decltype(list)>;

         surface_renderer_dsl_group() {
            if constexpr (std::is_same_v<T, VkDescriptorSet>)
               for (auto& item : list)
                  item = VK_NULL_HANDLE;
         }
         ~surface_renderer_dsl_group() {}
   };
   struct descriptor_set_layout_group : public surface_renderer_dsl_group<descriptor_set_layout> {
      using surface_renderer_dsl_group::surface_renderer_dsl_group;

      descriptor_set_layout_group() {}

      void setup_all(surface_renderer&);

      std::vector<VkDescriptorSetLayout> handles() const;
      std::vector<VkDescriptorPoolSize> needed_pool_sizes(size_t swap_chain_image_count) const;
   };
   struct descriptor_set_group : public surface_renderer_dsl_group<VkDescriptorSet> {
      using surface_renderer_dsl_group::surface_renderer_dsl_group;

      void allocate_all(surface_renderer&);
   };
}