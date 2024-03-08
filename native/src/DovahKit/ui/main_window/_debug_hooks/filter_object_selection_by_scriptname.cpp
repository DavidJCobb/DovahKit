#include "./filter_object_selection_by_scriptname.h"
#include "../../generic/FormPicker.h"
#include <QCheckBox>
#include <QDialog>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include "widgets/DKFormPicker.h"
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
      auto* form_picker  = new DKFormPicker(dialog);
      {
         QList<dovah::form_type> types;
         for (const auto& info : dovah::form_types) {
            if (info.is_reference())
               continue;
            using type_flag = dovah::form_type_info::flag::type;
            if (info.flags & (type_flag::no_editor_id | type_flag::no_connections | type_flag::is_singleton))
               continue;
            types.append(info.form_type);
         }
         form_picker->setAllowedFormTypes(types);
      }

      int row = 0;
      {
         auto* label = new QLabel("Required scriptname:", dialog);
         layout->addWidget(label, row, 0);

         auto* name = new QLineEdit(dialog);
         layout->addWidget(name, row, 1);
         label->setBuddy(name);

         QObject::connect(name, &QLineEdit::textEdited, dialog, [alias_picker, form_picker, ref_picker](const QString& text) {
            auto scriptname = text.toUtf8().toStdString();
            alias_picker->setRequiredScriptname(scriptname);
            form_picker->setRequiredScriptname(scriptname);
            ref_picker->setRequiredScriptname(scriptname);
         });
      }

      auto _add_divider = [&row, dialog, layout]() {
         auto* div = new QFrame(dialog);
         div->setFixedHeight(0);
         div->setFrameShape(QFrame::HLine);
         layout->addWidget(div, ++row, 0, 1, 2);
      };

      _add_divider();
      layout->addWidget(alias_picker, ++row, 0, 1, 2);
      _add_divider();
      layout->addWidget(ref_picker,   ++row, 0, 1, 2);
      _add_divider();
      layout->addWidget(form_picker,  ++row, 0, 1, 2);
      
      dialog->show();
   }
}