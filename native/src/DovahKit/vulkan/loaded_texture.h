#pragma once
#include <memory>
#include <QString>
#include "_vulkan.h"
#include "image.h"

#include "./scene_entities/base.h"
#include "./scene_entities/owned_gpu_resource_sets.h"
#include "./dds/texture.h"

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

      protected:
         struct queued_upload_info {
            image_metadata metadata;
            dds::texture   texture;
         };

      protected:
         std::unique_ptr<queued_upload_info> queued_upload;

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

         void prepare_for_gpu_upload(const image_metadata&, dds::texture&&);

         #pragma region Member functions for owned GPU resources (esp. for uploading)
         VkDeviceSize owned_gpu_resources_size() const noexcept;
         VkDeviceSize owned_gpu_resource_upload_alignment() const noexcept;
         void upload_owned_gpu_resources(scene_entities::owned_gpu_resource_upload_operation&);
         #pragma endregion

         void mark_for_delete();
         void reset();

         bool persist_for_life_of_renderer() const;
   };
}
