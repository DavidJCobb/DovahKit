#include "ui_bound_script.h"
#include <QDialog>
#include <QGridLayout>
#include <QKeySequence>
#include "dovah/forms/Form.h" // needed for loaded_form_ptr, at least for now
#include "widgets/DKObjectReferencePicker.h"
#include "widgets/DKPapyrusBoundScriptListPane.h"

namespace DovahKitDebug::features {
   class _test_dialog : public QDialog {
      public:
         _test_dialog(QWidget* parent) : QDialog(parent) {
            auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, this);

            auto* ref_picker = new DKObjectReferencePicker(this);
            auto* list_pane  = new DKPapyrusBoundScriptListPane(this);

            layout->addWidget(ref_picker);
            layout->addWidget(list_pane);

            QObject::connect(ref_picker, &DKObjectReferencePicker::refChanged, [this, list_pane](dovah::form_stub* stub) {
               if (!stub) {
                  this->loaded_form = nullptr;
                  return;
               }
               this->loaded_form = stub->load();
               if (!this->loaded_form)
                  return;
               list_pane->setFormWorkingCopy(this->loaded_form);
            });
         }

         dovah::loaded_form_ptr<dovah::loaded_forms::Form> loaded_form;
   };

   /*static*/ void ui_bound_script::execute(QWidget* from) {
      auto* dialog = new _test_dialog(from);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      dialog->show();
   }
}
