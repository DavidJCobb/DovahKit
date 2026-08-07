#include "./load_asset_as_sound_definition.h"
#include "dovah/files/bsa/bsa_archived_file.h"
#include "editor/subsystems/assets.h"
#include "../sound_definitions/fuz.h"
#include "../sound_definitions/wav.h"
#include "../sound_definitions/xwma.h"

namespace dovahkit::subsystems::audio::utils {
   extern std::shared_ptr<sound_definition> load_asset_as_sound_definition(std::filesystem::path path) {
      auto* data = dovahkit::subsystems::assets::get_or_create().lookup_game_asset(path, true);
      if (!data) {
         return nullptr;
      }

      //auto ext = path.extension();
      bool is_fuz = sound_definitions::fuz::data_is_likely_fuz(data->data(), data->size());
      bool is_xwm = false;
      bool is_wav = false;
      if (!is_fuz) {
         is_xwm = sound_definitions::xwma::data_is_likely_xwma(data->data(), data->size());
         if (!is_xwm)
            is_wav = sound_definitions::wav::data_is_likely_wav(data->data(), data->size());
      }

      if (!is_fuz && !is_xwm && !is_wav) {
         delete data;
         return nullptr;
      }

      std::unique_ptr<dovah::bsa_archived_file> data_ptr;
      data_ptr.reset(data);
      data = nullptr;
      if (is_fuz) {
         return std::make_shared<sound_definitions::fuz>(std::move(data_ptr));
      } else if (is_xwm) {
         return std::make_shared<sound_definitions::xwma>(std::move(data_ptr));
      } else if (is_wav) {
         return std::make_shared<sound_definitions::wav>(std::move(data_ptr));
      }
      return nullptr;
   }
}