#include "ui_bsa_picker.h"
#include "../../../dovah/files/bsa/bsa_load_order.h"
#include "../../../dovah/utils/get_ini_defined_bsa_list.h"
#include "../../../editor/core.h"
#include "../../../widgets/DKGameFilePicker.h"
#include "../../../widgets/widget-dialogs/DKBSABrowseDialog.h"
#include "../../../widgets/widget-models/DKBSACollectionModel.h"

namespace DovahKitDebug::features {
   /*static*/ void ui_bsa_picker::execute(QWidget* from) {
      qDebug("Loading BSAs...");
      dovah::bsa_load_order blo;
      {
         auto& editor = DovahKitCore::get();
         {
            auto  path   = std::filesystem::path();
            if (editor.has_data()) {
               editor.get_game_path(path, editor.get_current_game());
            } else {
               editor.get_game_path(path, dovah::game::skyrim_classic);
            }
            path.append("Data");
            blo.set_base_path(path);
         }
         auto list = dovah::utils::get_ini_defined_bsa_list(editor.get_current_game());
         for (const auto& path : list)
            blo.append_archive(path);
         blo.load_archives();
         blo.wait_for_archive_load_to_finish();
      }
      qDebug("BSAs loaded.");
      //
      qDebug("Filling BSA-to-Qt backend...");
      auto* backend = new DKBSACollectionModelBackend;
      backend->setArchives(blo);
      qDebug("Backend ready.");
      //
      //auto stem = "textures/";
      auto stem = "";
      auto path = DKBSABrowseDialog::getOpenFileName(
         from,
         QObject::tr("Test", "debug"),
         stem,
         "",
         QString(), // filter
         nullptr,   // selected filter
         {
            .backend = backend,
         }
      );
      qDebug("Selection: %s", path.toUtf8().constData());
      //
      delete backend;
   }
}
