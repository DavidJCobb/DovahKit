#include "./open_window_for_form_type.h"
#include "./core.h"
#include "ui/main_window.h" // MainWindow::get

#include "ui/form_group_windows/idle/IdleAnimationsDialog.h"

void open_edit_dialog_for_form_type(dovah::form_type type) {
   auto&    editor = DovahKitCore::get();
   auto*    parent = &MainWindow::get();
   QDialog* result = nullptr;
   switch (type) {
      case dovah::form_type::idle:
         result = editor.extant_form_type_dialogs.idle;
         if (!result)
            result = editor.extant_form_type_dialogs.idle = new IdleAnimationsDialog(parent);
         break;
   }
   if (result) {
      result->show();
      result->raise();
      result->activateWindow();
   }
}