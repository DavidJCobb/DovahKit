#include "./filter_object_selection_by_scriptname.h"
#include "../../generic/FormPicker.h"
#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include "widgets/DKQuestAliasPicker.h"
#include "widgets/DKObjectReferencePicker.h"

namespace DovahKitDebug::features {
   /*static*/ void filter_object_selection_by_scriptname::execute(QWidget* parent) {
      auto* dialog = new QDialog(parent);
      auto* layout = new QGridLayout(dialog);
      dialog->setLayout(layout);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

      auto* alias_picker = new DKQuestAliasPicker(dialog);
      auto* ref_picker   = new DKObjectReferencePicker(dialog);
      // TODO: DKFormPicker once we begin that refactor

      int row = 0;
      {
         auto* label = new QLabel("Required scriptname:", dialog);
         layout->addWidget(label, row, 0);

         auto* name = new QLineEdit(dialog);
         layout->addWidget(name, row, 1);
         label->setBuddy(name);

         QObject::connect(name, &QLineEdit::textEdited, dialog, [alias_picker, ref_picker](const QString& text) {
            auto scriptname = text.toUtf8().toStdString();
            ref_picker->setRequiredScriptname(scriptname);
         });
      }
      layout->addWidget(alias_picker, ++row, 0, 1, 2);
      layout->addWidget(ref_picker,   ++row, 0, 1, 2);
      
      dialog->show();
   }
}