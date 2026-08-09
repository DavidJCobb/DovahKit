#include "./load_asset_as_sound_definition.h"
#include "helpers/class_array.h"
#include "dovah/files/bsa/bsa_archived_file.h"
#include "editor/subsystems/assets.h"
#include "../exceptions/invalid_sound_file_data.h"
#include "../sound_definitions/fuz.h"
#include "../sound_definitions/wav.h"
#include "../sound_definitions/xwma.h"

namespace {
   namespace sound_definitions {
      using namespace dovahkit::subsystems::audio::sound_definitions;
   }
   using sound_definition_classes = cobb::class_array<
      sound_definitions::fuz,
      sound_definitions::xwma,
      sound_definitions::wav
   >;
}

namespace dovahkit::subsystems::audio::utils {
   extern std::shared_ptr<sound_definition> load_asset_as_sound_definition(std::filesystem::path path) {
      auto* file = dovahkit::subsystems::assets::get_or_create().lookup_game_asset(path, true);
      if (!file) {
         auto ext = path.extension();
         if (ext == ".wav") {
            //
            // For whatever reason, data files specify `.wav` for several files including 
            // all music, but the actual files in the BSA are `.xwm`.
            //
            path.replace_extension(".xwm");
            file = dovahkit::subsystems::assets::get_or_create().lookup_game_asset(path, true);
         }
         if (!file)
            return nullptr;
      }

      std::shared_ptr<sound_definition> result;
      {
         const auto* file_data = file->data();
         const auto  file_size = file->size();
         sound_definition_classes::for_each_until_true([&file, file_data, file_size, &result]<typename T>() {
            if (T::is_valid_data(file_data, file_size)) {
               std::unique_ptr<dovah::bsa_archived_file> file_ptr;
               file_ptr.reset(file);
               file = nullptr;
               try {
                  result = std::make_shared<T>(std::move(file_ptr));
               } catch (exceptions::invalid_sound_file_data& ex) {
                  result.reset();
               }
               return true;
            }
            return false;
         });
      }
      if (file) {
         delete file;
         file = nullptr;
      }
      return result;
   }
}