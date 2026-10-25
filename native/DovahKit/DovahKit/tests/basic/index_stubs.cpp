#include "./index_stubs.h"
#include <filesystem>
#include <stdexcept>
#include <unordered_map>
#include <QInputDialog>
#include "dovah/callback_sets/data_load.h"
#include "dovah/tes/file.h"
#include "dovah/tes/file_load_order.h"
#include "dovah/tes/file_queue.h"
#include "dovah/active_load_order.h"

namespace dovahkit::tests {
   static void on_warning(const dovah::notices::base_warning& warning) {
      __debugbreak();
   }

   extern void index_stubs() {
      bool ok = false;
      QString data = QInputDialog::getText(
         nullptr,
         "Enter SSE data directory",
         "",
         {},
         "",
         &ok
      );
      if (!ok)
         return;

      std::filesystem::path data_directory = data.toStdString();

      dovah::tes::file_queue queue;
      queue.insert("Skyrim.esm");

      
      dovah::tes::file_load_order flo;
      flo.config.data_directory = data_directory;
      flo.config.target_game    = dovah::game::skyrim_special;
      try {
         flo.add(queue);
      } catch ([[maybe_unused]] const std::exception& e) {
         #if _DEBUG
            __debugbreak();
         #endif
         return;
      } catch (...) {
         #if _DEBUG
            __debugbreak();
         #endif
         return;
      }

      dovah::active_load_order alo;
      alo.set_data_directory(data_directory);
      try {
         alo.load(
            flo,
            {},
            dovah::callback_sets::data_load{
               .on_warning = &on_warning,
            }
         );
      } catch ([[maybe_unused]] std::exception& e) {
         __debugbreak();
         return;
      } catch (...) {
         __debugbreak();
         return;
      }

      // Load done.
      __debugbreak();
      
      {  // TEST: Hardcoded: Player
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x7);
         __debugbreak();
      }
      {  // TEST: Hardcoded: PlayerRef
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x14);
         __debugbreak();
      }
      {  // TEST: [CELL:000090E3] (7, 7) in [WRLD:03C]Tamriel
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x90E3);
         __debugbreak();
      }
      {  // TEST: [REFR:0001A6C5]FrukiREF persistent child of [CELL:00013A81]DawnstarFrukisHouse
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x0001A6C5);
         __debugbreak();
      }
      {  // TEST: [DIAL:00013622]FFRiften10SharedInfos
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x00013622);
         __debugbreak();
      }
      {  // TEST: [INFO:0001362A]FFRiften10SharedTopic01 in [DIAL:00013622]FFRiften10SharedInfos
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x0001362A);
         __debugbreak();
      }
   }
}