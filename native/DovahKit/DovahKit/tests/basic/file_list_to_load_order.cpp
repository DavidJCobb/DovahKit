#include "./file_list_to_load_order.h"
#include <filesystem>
#include <QInputDialog>
#include <QMessageBox>
#include "dovah/tes/file_load_order.h"
#include "dovah/tes/file_queue.h"
#include "dovah/exceptions/file_load_error.h"
#include "dovah/exceptions/load_order_error.h"
#pragma region Errors to strings
   #include "editor/localize/exceptions/file_load_error.h"
   #include "editor/localize/exceptions/load_order_error.h"
#pragma endregion

namespace dovahkit::tests {
   extern void file_list_to_load_order() {
      bool ok = false;
      QString data = QInputDialog::getText(
         nullptr,
         "Enter data directory",
         "",
         {},
         "",
         &ok
      );
      if (!ok)
         return;

      std::filesystem::path data_directory = data.toStdString();

      ok = false;
      QString files = QInputDialog::getMultiLineText(
         nullptr,
         "Enter files",
         "One filename per line",
         "",
         &ok
      );
      if (!ok)
         return;

      dovah::tes::file_queue queue;
      {
         auto list = files.split("\n");
         for (const auto filename : list) {
            queue.insert(filename.toStdString());
         }
      }

      dovah::tes::file_load_order flo;
      flo.config.data_directory = std::move(data_directory);
      try {
         flo.add(queue);
      } catch (const dovah::exceptions::file_load_error& e) {
         QMessageBox::critical(nullptr, "Error", dovahkit::localize::exceptions::file_load_error(e));
         #if _DEBUG
            __debugbreak();
         #endif
         return;
      } catch (const dovah::exceptions::load_order_error& e) {
         QMessageBox::critical(nullptr, "Error", dovahkit::localize::exceptions::load_order_error(e));
         #if _DEBUG
            __debugbreak();
         #endif
         return;
      } catch (const std::exception& e) {
         const auto* what = e.what();
         if (what) {
            QMessageBox::critical(nullptr, "Error", QString("Exception thrown: %1").arg(what));
         } else {
            QMessageBox::critical(nullptr, "Error", "Exception thrown");
         }
         #if _DEBUG
            __debugbreak();
         #endif
         return;
      } catch (...) {
         QMessageBox::critical(nullptr, "Error", "Unknown value thrown");
         #if _DEBUG
            __debugbreak();
         #endif
         return;
      }
      
      QString out_masters;
      for (const auto& info : flo.masters) {
         out_masters += QString::fromStdString(info->filename);
         if (info->is_light()) {
            out_masters += " (light)";
         }
         out_masters += "\n";
      }
      QString out_plugins;
      for (const auto& info : flo.plugins) {
         out_plugins += QString::fromStdString(info->filename);
         if (info->is_light()) {
            out_plugins += " (light)";
         }
         out_plugins += "\n";
      }

      QMessageBox::information(
         nullptr,
         "Result",
         QString("Masters:\n%1\nPlugins:\n%2").arg(out_masters).arg(out_plugins)
      );
   }
}