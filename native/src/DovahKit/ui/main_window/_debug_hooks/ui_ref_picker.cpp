#include "ui_ref_picker.h"
#include <QDialog>
#include <QGridLayout>
#include "widgets/DKObjectReferencePicker.h"
#include "dovah/form_stub.h"

namespace DovahKitDebug::features {
   /*static*/ void ui_ref_picker::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      auto* widget = new DKObjectReferencePicker(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      
      widget->setAllowNone(true);

      layout->addWidget(widget);
      QObject::connect(widget, &DKObjectReferencePicker::refChanged, widget, [widget](dovah::form_stub* stub) {
         if (!stub)
            qDebug("Stub: NONE");
         else
            qDebug("Stub: %08X (%s)", stub->formID, stub->editorID.c_str());
      });
      
      dialog->show();
   }
}
