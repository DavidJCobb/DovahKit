#include "worker_thread_for_nifs.h"
#include "../rendered_nif.h"
#include "../surface_renderer.h"

#include "editor/subsystems/crash_dumper/register_new_thread.h"

#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/utils/asset_paths/scope_to_folder.h"
#include "editor/subsystems/assets.h"

namespace vulkanDK::asset_loading {
   void worker_thread_for_nifs::_load_single_nif(queued_nif_load& item) {
      assert(item.nif);
      assert(item.form_data.loaded_form);
      assert(item.form_data.model);
      auto& nif   = *item.nif;
      auto& model = *item.form_data.model;

      nif.multi_thread_state.flags |= rendered_nif::loading_flag::loading;
      if (model.model_path.empty()) {
         nif.multi_thread_state.flags ^= (rendered_nif::loading_flag::loading | rendered_nif::loading_flag::load_canceled);
         return;
      }

      std::filesystem::path normalized_path = dovah::utils::asset_paths::scope_to_meshes_folder<std::filesystem::path>(model.model_path);
      
      std::unique_ptr<dovah::bsa_archived_file> file(dovahkit::subsystems::assets::get().lookup_game_asset(normalized_path));
      if (!file) {
         qDebug("Failed to open NIF file: <%s>", normalized_path.string().c_str());
         nif.multi_thread_state.flags ^= (rendered_nif::loading_flag::loading | rendered_nif::loading_flag::load_failure);
         return;
      }

      nif.read((void*)file->data(), file->size());
      auto& error = nif.read_error();
      if (error.code != nifDK::default_notice_code) {
         qDebug("Failed to parse NIF file: <%s>\n - Error code %08X.", normalized_path.string().c_str(), error.code);
         #if _DEBUG
            __debugbreak();
         #endif
         nif.multi_thread_state.flags ^= (rendered_nif::loading_flag::loading | rendered_nif::loading_flag::load_failure);
         return;
      }

      nif.multi_thread_state.flags ^= (rendered_nif::loading_flag::loading | rendered_nif::loading_flag::load_success);
   }

   void worker_thread_for_nifs::run() {
      dovahkit::subsystems::crash_dumper::register_new_thread();
      auto& list = this->owner.loading.meshes.batches[this->index];
      for (auto& item : list) {
         if (item.nif->is_cancel_requested()) {
            item.nif->multi_thread_state.flags |= rendered_nif::loading_flag::load_canceled;
            continue;
         }
         this->_load_single_nif(item);
      }
   }
}