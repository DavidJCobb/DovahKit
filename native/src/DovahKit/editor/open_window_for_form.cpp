#include "open_window_for_form.h"
#include <QMessageBox>
#include "../dovah/form_stub.h"
#include "../ui/form_windows/color.h"
#include "../ui/form_windows/shout.h"
#include "../ui/form_windows/voicetype.h"
#include "../ui/form_windows/word_of_power.h"

void open_window_for_form(dovah::form_stub* stub, QWidget* parent) {
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