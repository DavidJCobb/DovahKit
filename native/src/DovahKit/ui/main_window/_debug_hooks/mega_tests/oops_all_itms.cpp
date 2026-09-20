#include "./oops_all_itms.h"
#include <QInputDialog>
#include <QMessageBox>
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/files/tes_file_reading/file_loader.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

namespace DovahKitDebug::features::mega_tests {
   /*
      Enter a filename, and this will find all forms defined or edited by that file 
      and flag them as edited.

      The intended use is to create an active file consisting solely of re-saved 
      forms from one of its masters. You would then load that active file in xEdit 
      and examine its records: if absolutely everything is an ITM, then we know we 
      loaded and re-saved those records properly. If any changes are present, then 
      those changes need to be manually examined to see whether they're no-ops; if 
      the changes are substantial, that would imply that we re-saved something in 
      an incorrect way.
   */
   /*static*/ void oops_all_itms::execute(QWidget* from) {
      auto filename = QInputDialog::getText(from, "Enter filename)", "Filename including extension (case-insensitive; blank = cancel):");
      if (filename.isEmpty())
         return;

      std::optional<dovah::file_prefix> prefix;
      const dovah::file_load_order::loaded_file* desired_file = nullptr;

      auto& editor = DovahKitCore::get();
      auto* flo    = editor.get_file_load_order();
      auto  files  = editor.get_loaded_files();
      for (auto* file : files) {
         auto name = file->get_filename();
         if (QString::fromStdString(name).compare(filename, Qt::CaseInsensitive) == 0) {
            desired_file = file;
            prefix       = flo->file_prefix_for(*file);
            break;
         }
      }
      if (!prefix.has_value()) {
         QMessageBox::critical(from, "Error", "File not found in current load order");
         return;
      }
      
      //
      // NOTE: This will miss GMSTs. This is because while we can flag their stubs as edited, 
      // DovahKit's backend doesn't care that much about the stubs. The actual values of the 
      // GMSTs are stored elsewhere (since GMSTs aren't "real forms").
      // 
      // The only way to correlate a loaded game setting to its source stubs (yes, possibly 
      // multiple) and thus to its source files is via the `loaded_game_setting` objects. The 
      // file-load-order does maintain one `loaded_game_setting` per source stub, but it only 
      // allows the outside world to access the last-loaded one.
      // 
      // Thus we can see the stubs, but we don't know what game settings they define; and we 
      // can see only the last-loaded game setting; so we can't tell, from out here, whether 
      // a given GMST was defined or overridden in a particular file unless that file was the 
      // winning definition. Changing this would require editing the API that the FLO provides, 
      // and I don't want to have to recompile the entire program just for this. But since a 
      // GMST is so simple, I don't think we need the "Oops! All ITMs" test to know that we 
      // handle them properly.
      //

      size_t count = 0;
      editor.for_each_form([&prefix, desired_file, &count](dovah::form_stub* form) -> bool {
         if (form->is_none_stub())
            return false;
         if (form->form_type == dovah::form_type::none) // don't try to save PapyrusPersistenceForm, etc.; we can't
            return false;
         if (form->formID == dovah::hardcoded_form_ids::PlayerRef) // PlayerRef can't be saved because it isn't in a cell, so don't flag it as edited
            return false;

         if (prefix.value().contains_form_id(form->formID)) {
            form->set_edited(true);
            ++count;
            return false;
         }

         // Check for overrides
         auto sfc = form->source_file_count();
         if (sfc > 1) {
            for (size_t i = 1; i < sfc; ++i) {
               auto* info = form->get_source_file_info(i);
               if (!info)
                  break;
               auto* file = info->pointer;
               if (file == desired_file) { // overridden by target file
                  form->set_edited(true);
                  ++count;
                  return false;
               }
            }
         }

         return false;
      });
      QMessageBox::information(from, "Done", QString("Flagged %1 forms as edited.").arg(count));
   }
}
