#include "./load_a_shout.h"
#include <filesystem>
#include <stdexcept>
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

   extern void load_a_shout() {
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
      alo.get_or_create_bsa_collection();
      try {
         alo.load(
            flo,
            {}
         );
      } catch ([[maybe_unused]] std::exception& e) {
         __debugbreak();
         return;
      } catch (...) {
         __debugbreak();
         return;
      }
      alo.post_load_callbacks = {
         .on_warning = &on_warning,
      };
      
      {  // Become Ethereal
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x32920);
         auto  ptr  = stub->load();
         __debugbreak();
      }
      {  // Dismay
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x2395a);
         auto  ptr  = stub->load();
         __debugbreak();
      }
   }
}