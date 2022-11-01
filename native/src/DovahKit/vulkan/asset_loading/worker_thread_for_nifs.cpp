#include "worker_thread_for_nifs.h"
#include "../rendered_nif.h"
#include "../surface_renderer.h"

#include "dovah/files/bsa/bsa_archived_file.h"
#include "editor/subsystems/assets.h"

namespace vulkanDK::asset_loading {
   void worker_thread_for_nifs::_load_single_nif(queued_nif_load& item) {
      assert(item.nif);
      assert(item.form_data.loaded_form);
      assert(item.form_data.model);
      auto& nif   = *item.nif;
      auto& model = *item.form_data.model;

      nif.multi_thread_state.flags |= rendered_nif::loading_flag::loading;

      std::filesystem::path path = std::string("meshes") + (model.model_path[0] == '/' || model.model_path[0] == '\\' ? "" : "\\") + model.model_path;
      //
      std::unique_ptr<dovah::bsa_archived_file> file(dovahkit::subsystems::assets::get().lookup_game_asset(path));
      if (!file) {
         qDebug("Failed to open NIF file: <%s>", path.string().c_str());
         nif.multi_thread_state.flags ^= (rendered_nif::loading_flag::loading | rendered_nif::loading_flag::load_failure);
         return;
      }

      nif.read((void*)file->data(), file->size());
      auto& error = nif.read_error();
      if (error.code != nifDK::default_notice_code) {
         qDebug("Failed to parse NIF file: <%s>\n - Error code %08X.", path.string().c_str(), error.code);
         #if _DEBUG
            __debugbreak();
         #endif
         nif.multi_thread_state.flags ^= (rendered_nif::loading_flag::loading | rendered_nif::loading_flag::load_failure);
         return;
      }

      nif.multi_thread_state.flags ^= (rendered_nif::loading_flag::loading | rendered_nif::loading_flag::load_success);
   }

   void worker_thread_for_nifs::run() {
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