#pragma once
#include <QString>
#include "_vulkan.h"
#include "helpers/frame_dirty_state.h"
#include "image.h"
#include "scene_frame_item.h"

#include "./scene_entities/base.h"
#include "./scene_entities/owned_gpu_resource_sets.h"

namespace vulkanDK {
   struct loaded_texture : public scene_entities::base {
      public:
         static constexpr const char* name_single = "texture";
         static constexpr const char* name_plural = "textures";
         //
         static constexpr const bool owned_gpu_resources_are_coalesced   = false;
         static constexpr const bool owned_gpu_resources_are_descriptors = true;
         static constexpr const bool is_drawn = false;
      public:
         struct flag { // flags applied on the CPU, not within shaders
            enum type : uint32_t {
               is_default_land_texture = 0x00000001,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

      public:
         flags_t flags = 0;
         //
         scene_entities::owned_gpu_resource_sets<owned_image_and_view> owned_gpu_resources;
         //
         uint32_t w = 0;
         uint32_t h = 0;
         QString  path;
         //
         uint32_t refcount = 0;

         void mark_for_delete();
         void reset();

         bool persist_for_life_of_renderer() const;
   };
}
