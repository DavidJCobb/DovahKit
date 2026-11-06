#include "./load_skyrim_esm.h"
#include <filesystem>
#include <stdexcept>
#include <QInputDialog>
#include "dovah/callback_sets/data_load.h"
#include "dovah/tes/file.h"
#include "dovah/tes/file_load_order.h"
#include "dovah/tes/file_queue.h"
#include "dovah/active_load_order.h"

namespace dovahkit::tests::pieces {
   static void break_on_warning(const dovah::notices::base_warning& warning) {
      __debugbreak();
   }

   std::unique_ptr<dovah::active_load_order> load_skyrim_esm(bool break_on_warnings) {
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
         return nullptr;

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
         return nullptr;
      } catch (...) {
         #if _DEBUG
            __debugbreak();
         #endif
         return nullptr;
      }

      auto  alo_ptr = std::make_unique<dovah::active_load_order>();
      auto& alo = *alo_ptr;
      alo.set_data_directory(data_directory);
      alo.get_or_create_bsa_collection();
      try {
         alo.load(
            flo,
            {
               .on_warning = break_on_warnings ? &break_on_warning : nullptr,
            }
         );
      } catch ([[maybe_unused]] std::exception& e) {
         __debugbreak();
         return nullptr;
      } catch (...) {
         __debugbreak();
         return nullptr;
      }
      alo.post_load_callbacks = {
         .on_warning = break_on_warnings ? &break_on_warning : nullptr,
      };

      return std::move(alo_ptr);
   }
}