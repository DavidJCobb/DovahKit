#pragma once
#include <array>
#include <type_traits>
#include "_vulkan.h"
#include "config/scene_limits.h"
#include "descriptor_definitions.h"

namespace vulkanDK {
   class surface_renderer;

   template<typename T> requires (std::is_same_v<T, VkDescriptorSet> || std::is_same_v<T, descriptor_set_layout>)
   struct surface_renderer_dsl_group {
      public:
         union {
            std::array<T, 8> list = {};
            struct {
               T oit_composite;
               T sun_shadows;
               T light_shadows;
               T standard;
               T fps;
               T world_axes;
               T scene_bounds;
               T landscape;
            };
         };
         static constexpr size_t size = std::tuple_size_v<decltype(list)>;

         surface_renderer_dsl_group() {}
         ~surface_renderer_dsl_group() {}
   };

   struct descriptor_set_layout_group : public surface_renderer_dsl_group<descriptor_set_layout> {
      using surface_renderer_dsl_group::surface_renderer_dsl_group;
      protected:
         void _needed_pool_sizes_for(std::vector<VkDescriptorPoolSize>&, const descriptor_set_layout&, size_t using_set_count = 1) const;

         template<size_t S> class layout_with_size : public descriptor_set_layout {
            public:
               using descriptor_set_layout::descriptor_set_layout;

               static constexpr size_t using_set_count = S;
         };
      public:
         template<typename U> static size_t using_set_count_of(const U& x) { return U::using_set_count; }

      public:
         union _shared_layout_union {
            //
            // Layouts used by more than one set. Must match the order of the using sets.
            //
            std::array<descriptor_set_layout, 2> list;
            struct {
               layout_with_size<2> compute_frustum_culling;
                  // - compute_frustum_culling_main
                  // - compute_frustum_culling_sun
               layout_with_size<config::max_active_shadow_casters> compute_shadow_caster_culling;
                  // - compute_shadow_caster_culls[i]
            };

            using list_t = std::remove_reference_t<decltype(list)>;
            _shared_layout_union() : list({}) {}
            ~_shared_layout_union() { list.~list_t(); }
         } shared_layouts;

         void setup_all(surface_renderer&);
         void teardown_all();

         std::vector<VkDescriptorSetLayout> handles() const;
         std::vector<VkDescriptorSetLayout> handles_for_sets() const; // includes duplicates for shared layouts
         std::vector<VkDescriptorPoolSize> needed_pool_sizes() const;
         std::vector<uint32_t> variable_binding_counts() const;

         size_t total_set_count() const;
   };

   struct descriptor_set_group : public surface_renderer_dsl_group<VkDescriptorSet> {
      using surface_renderer_dsl_group::surface_renderer_dsl_group;

      union {
         //
         // Sets that share a layout.
         //
         std::array<VkDescriptorSet, 2 + config::max_active_shadow_casters> list = {};
         struct {
            VkDescriptorSet compute_frustum_culling_main;
            VkDescriptorSet compute_frustum_culling_sun;
            std::array<VkDescriptorSet, config::max_active_shadow_casters> compute_shadow_caster_culls;
         };
      } sharing_sets;

      descriptor_set_group() {
         for (auto& item : list)
            item = VK_NULL_HANDLE;
         for (auto& item : sharing_sets.list)
            item = VK_NULL_HANDLE;
      }

      void allocate_all(surface_renderer&);
      void free_all(surface_renderer&);
   };
}