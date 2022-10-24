#include "loaded_texture.h"
#include <stdexcept>
#include <QString>
#include "./scene_entities/owned_gpu_resource_upload_operation.h"
#include "./data/vulkan_formats.h"

namespace vulkanDK {
   void loaded_texture::prepare_for_gpu_upload(const image_metadata& md, dds::texture&& tex) {
      this->queued_upload = std::make_unique<queued_upload_info>();
      this->queued_upload->metadata = md;
      this->queued_upload->texture  = std::move(tex);
   }

   #pragma region Member functions for owned GPU resources (esp. for uploading)
   VkDeviceSize loaded_texture::owned_gpu_resources_size() const noexcept {
      if (!this->queued_upload)
         return 0;
      return this->queued_upload->texture.pixel_data_size();
   }
   VkDeviceSize loaded_texture::owned_gpu_resource_upload_alignment() const noexcept {
      if (!this->queued_upload)
         return 0;
      auto format = this->queued_upload->metadata.format;
      for (const auto& data : data::all_vulkan_formats) {
         if (data.format > format)
            break;
         if (data.format == format)
            return data.texel_block_size;
      }
      return 16;
   }
   void loaded_texture::upload_owned_gpu_resources(scene_entities::owned_gpu_resource_upload_operation& upload) {
      assert(this->queued_upload != nullptr);
      const auto& qu = *this->queued_upload.get();

      upload.stage_data(qu.texture.pixel_data(), qu.texture.pixel_data_size());

      this->owned_gpu_resources.current = upload.create_image_and_view();
      auto& image = this->owned_gpu_resources.current;
      image.metadata       = qu.metadata;
      image.metadata.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
      this->queued_upload.reset();
      try {
         image.create_image(image.metadata, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
         image.create_basic_view(image.metadata.format, VK_IMAGE_ASPECT_COLOR_BIT);
         #if _DEBUG
            upload.set_debug_object_name(image.handle, QString("2D Image <Tex %1> <%2>").arg(upload.get_entity_index()).arg(this->path).toStdString());
            upload.set_debug_object_name(image.view,   QString("Image View <Tex %1> <%2>").arg(upload.get_entity_index()).arg(this->path).toStdString());
         #endif
         upload.queue_upload_to_image(image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);
      } catch (std::runtime_error& e) {
         qDebug("[vulkanDK::loaded_texture::owned_gpu_resources_size] Exception thrown while trying to upload to the GPU.");
         //this->owned_gpu_resources.current.teardown();
         //this->mark_for_delete();
      }
      this->queued_upload = nullptr;
   }
   #pragma endregion

   void loaded_texture::mark_for_delete() {
      base::_mark_for_delete<loaded_texture>();
      if (this->owned_gpu_resources.has_current()) {
         //
         // A pending-delete entity will have both "current" and "outdated" resources 
         // if it was marked for delete after recycling began, but before recycling 
         // could complete. Any such entity should be considered irrecoverable, so 
         // let's clear out our texture path and other data so that we get skipped 
         // more quickly when attempts are made to look up and reuse a loaded texture 
         // by file path (`scene::reuse_scene_texture`).
         //
         this->w = 0;
         this->h = 0;
         this->path.clear();
      }
   }
   void loaded_texture::reset() {
      base::_reset<loaded_texture>();
      this->w = 0;
      this->h = 0;
      this->path.clear();
      this->queued_upload = nullptr;
   }

   bool loaded_texture::persist_for_life_of_renderer() const {
      return (this->flags & (flag::is_default_land_texture)) != 0;
   }
}