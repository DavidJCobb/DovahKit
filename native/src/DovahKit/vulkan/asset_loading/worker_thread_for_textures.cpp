#include "worker_thread_for_textures.h"
#include "../loaded_texture.h"
#include "../surface_renderer.h"

#include "editor/subsystems/crash_dumper/register_new_thread.h"

#include "dovah/files/bsa/bsa_archived_file.h"
#include "editor/subsystems/assets.h"

namespace vulkanDK::asset_loading {
   namespace {
      void _fail_texture_load(surface_renderer& sr, loaded_texture& entity) {
         //
         // Write in error data: a single purple pixel.
         //
         constexpr const auto error_pixel_data = std::array{
            (uint8_t)255,
            (uint8_t)0,
            (uint8_t)255,
            (uint8_t)255,
         };
         auto error = dds::texture::from_r8g8b8a8(error_pixel_data.data(), error_pixel_data.size(), 1, 1);
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

      // TODO: Revise this post-launch. This is a hazard; `dds::texture` owns its buffer, so we 
      //       are here giving it ownership of a buffer that is already owned (by the returned 
      //       file). Both of them are fighting over ownership and we have to do too much to 
      //       manually manage which of them gets to win at any given moment.
      //
      //       We should instead have a class that parses a DDS buffer into the information we 
      //       require without requiring ownership of it; and when we decide that we do need to 
      //       keep and own the data, we should have a more explicit hand-off at exactly that 
      //       moment.
      //
      dds::texture tex;
      tex.data = file->data();
      tex.size = file->size();

      if (!tex.read()) {
         qDebug("[vulkanDK::surface_renderer::add_dds_texture] Failed to read DDS header: %s", qUtf8Printable(entity.path));
         tex.data = nullptr; // ensure we don't double-free (from `tex` and implicitly from `file`)
         tex.size = 0;
         _fail_texture_load(this->owner, entity);
         return;
      }
      if (!tex.pixel_data() || !tex.pixel_data_size()) {
         qDebug("[vulkanDK::surface_renderer::add_dds_texture] No DDS data available: %s", qUtf8Printable(entity.path));
         tex.data = nullptr; // ensure we don't double-free (from `tex` and implicitly from `file`)
         tex.size = 0;
         _fail_texture_load(this->owner, entity);
         return;
      }
      const auto vulkan_metadata = image_metadata::from_dds_header(tex.metadata, tex.pixel_data_size());
      if (vulkan_metadata.format == VkFormat::VK_FORMAT_UNDEFINED) {
         qDebug("[vulkanDK::surface_renderer::add_dds_texture] DDS texture format did not map to Vulkan: %s", qUtf8Printable(entity.path));
         tex.data = nullptr; // ensure we don't double-free (from `tex` and implicitly from `file`)
         tex.size = 0;
         _fail_texture_load(this->owner, entity);
         return;
      }

      entity.w = tex.metadata.width;
      entity.h = tex.metadata.height;
      //
      // Queue transfer to the GPU:
      // 
      // Right now, `tex` is borrowing a buffer directly from the BSA-archived file. If that 
      // buffer is shared, then we have to copy it; otherwise, we can simply steal it.
      //
      if (file->is_shared()) {
         auto* copy = malloc(tex.size);
         if (!copy) {
            qDebug("[vulkanDK::surface_renderer::add_dds_texture] DDS texture too large to create an owned copy: %s", qUtf8Printable(entity.path));
            tex.data = nullptr; // ensure we don't double-free (from `tex` and implicitly from `file`)
            tex.size = 0;
            _fail_texture_load(this->owner, entity);
            return;
         }
         memcpy(copy, tex.data, tex.size);
         tex.data = copy;
      } else {
         //
         // This function call detaches the buffer from `file`. Ordinarily we'd store the 
         // pointer and size that it returns, but as it happens, we've already stored those 
         // values in `tex` further above.
         //
         file->take_owned_data().take();
      }
      //
      entity.lifetime.life_state = scene_entities::life_state::active_pending_upload;
      entity.lifetime.sync_state.set_all_out_of_date();
      entity.prepare_for_gpu_upload(vulkan_metadata, std::move(tex));
      ++this->owner.uploading.pending_upload_counts.value_for<loaded_texture>();
   }

   void worker_thread_for_textures::run() {
      dovahkit::subsystems::crash_dumper::register_new_thread();
      auto& list  = this->owner.scene.entities_of_type<loaded_texture>();
      auto& queue = this->owner.loading.textures.batches[this->index];
      for (auto index : queue) {
         this->_load_single_texture(list[index]);
      }
   }
}