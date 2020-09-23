#include "open_window_for_form.h"
#include <QMessageBox>
#include "../dovah/form_stub.h"
#include "core.h"
#include "../ui/form_windows/color.h"
#include "../ui/form_windows/shout.h"
#include "../ui/form_windows/voicetype.h"
#include "../ui/form_windows/word_of_power.h"

void open_window_for_form(dovah::form_stub* stub, QWidget* parent) {
   //
   // First, let's check if there's already a window for this form. If so, we should just 
   // refocus that window instead of opening a new one.
   //
   auto  formID = stub->formID;
   auto& editor = DovahKitCore::get();
   auto  it     = editor.extant_form_edit_dialogs.find(formID);
   if (it != editor.extant_form_edit_dialogs.end()) {
      auto dialog = it->second;
      if (dialog) {
         dialog->raise();
         dialog->activateWindow();
         return;
      }
   }
   //
   // If we made it to here, then there isn't already a window for this form, so let's 
   // open one.
   //
   QDialog* opened = nullptr;
   switch (stub->formType) {
      case dovah::form_type::color:
         opened = new FormDialogColor(stub, parent);
         break;
      case dovah::form_type::shout:
         opened = new FormDialogShout(stub, parent);
         break;
      case dovah::form_type::voicetype:
         opened = new FormDialogVoicetype(stub, parent);
         break;
      case dovah::form_type::word_of_power:
         opened = new FormDialogWordOfPower(stub, parent);
         break;
   }
   if (opened) {
      editor.extant_form_edit_dialogs[stub->formID] = opened;
      QObject::connect(opened, &QDialog::finished, &editor, [formID, opened]() {
         auto& editor = DovahKitCore::get();
         auto& map    = editor.extant_form_edit_dialogs;
         auto  it     = map.find(formID);
         if (it != map.end())
            map.erase(it);
         //
         opened->deleteLater();
      });
      //
      opened->show();
      return;
   }
   //
   auto& info = dovah::form_type_info::lookup(stub->formType);
   QString title = QObject::tr("Error: cannot edit %1");
   if (&info == &dovah::form_types[0])
      title = title.arg("unknown type");
   else
      title = title.arg(info.name);
   QMessageBox::information(parent, title, QObject::tr("DovahKit doesn't yet support editing this form type."));
}