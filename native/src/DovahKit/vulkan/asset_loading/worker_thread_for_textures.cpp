#include "worker_thread_for_textures.h"
#include "../loaded_texture.h"
#include "../surface_renderer.h"

#include "dovah/files/bsa/bsa_archived_file.h"
#include "editor/subsystems/assets.h"

namespace vulkanDK::asset_loading {
   namespace {
      void _fail_texture_load(surface_renderer& sr, loaded_texture& entity) {
         //
         // Write in error data: a single purple pixel.
         //
         dds::texture error;
         error.data = malloc(4);
         ((uint8_t*)error.data)[0] = 255;
         ((uint8_t*)error.data)[1] = 0;
         ((uint8_t*)error.data)[2] = 255;
         ((uint8_t*)error.data)[3] = 255;
         //
         auto vulkan_metadata = image_metadata{
            .extent = { .width = 1, .height = 1, .depth = 1 },
            .format = VkFormat::VK_FORMAT_R8G8B8A8_UINT,
         };
         //
         entity.lifetime.life_state = scene_entities::life_state::active_pending_upload;
         entity.lifetime.sync_state.set_all_out_of_date();
         entity.prepare_for_gpu_upload(vulkan_metadata, std::move(error));
         ++sr.uploading.pending_upload_counts.value_for<loaded_texture>();
      }
   }

   void worker_thread_for_textures::_load_single_texture(loaded_texture& entity) {
      std::unique_ptr<dovah::bsa_archived_file> file;
      std::filesystem::path path = entity.path.toStdWString();
      file.reset(dovahkit::subsystems::assets::get_or_create().lookup_game_asset(path)); // TODO: switch to `get` once we're sure the asset subsystem is constructed elsewhere
      if (!file) {
         qDebug("[vulkanDK::surface_renderer::add_dds_texture] Failed to open texture: %s", qUtf8Printable(entity.path));
         _fail_texture_load(this->owner, entity);
         return;
      }

      dds::texture tex;
      tex.data = file->data();
      tex.size = file->size();
      //
      if (!tex.read()) {
         qDebug("[vulkanDK::surface_renderer::add_dds_texture] Failed to read DDS header: %s", qUtf8Printable(entity.path));
         _fail_texture_load(this->owner, entity);
         return;
      }
      if (!tex.pixel_data() || !tex.pixel_data_size()) {
         qDebug("[vulkanDK::surface_renderer::add_dds_texture] No DDS data available: %s", qUtf8Printable(entity.path));
         _fail_texture_load(this->owner, entity);
         return;
      }
      const auto vulkan_metadata = image_metadata::from_dds_header(tex.metadata, tex.pixel_data_size());
      if (vulkan_metadata.format == VkFormat::VK_FORMAT_UNDEFINED) {
         qDebug("[vulkanDK::surface_renderer::add_dds_texture] DDS texture format did not map to Vulkan: %s", qUtf8Printable(entity.path));
         _fail_texture_load(this->owner, entity);
         return;
      }

      entity.w = tex.metadata.width;
      entity.h = tex.metadata.height;
      //
      // Queue transfer to the GPU:
      // 
      // Right now, `tex` is borrowing a buffer directly from the BSA-archived file, and that 
      // buffer's gonna get deleted when we're done with the BSA data. We can't simply "steal" 
      // it from the BSA-archived file object, because it may actually be shared with the BSA 
      // itself (i.e. if the file is uncompressed). We have to instead just copy the buffer.
      //
      auto* copy = malloc(tex.size);
      memcpy(copy, tex.data, tex.size);
      tex.data = copy;
      //
      entity.lifetime.life_state = scene_entities::life_state::active_pending_upload;
      entity.lifetime.sync_state.set_all_out_of_date();
      entity.prepare_for_gpu_upload(vulkan_metadata, std::move(tex));
      ++this->owner.uploading.pending_upload_counts.value_for<loaded_texture>();
   }

   void worker_thread_for_textures::run() {
      auto& list  = this->owner.scene.entities_of_type<loaded_texture>();
      auto& queue = this->owner.loading.texture_batches[this->index];
      for (auto index : queue) {
         this->_load_single_texture(list[index]);
      }
   }
}