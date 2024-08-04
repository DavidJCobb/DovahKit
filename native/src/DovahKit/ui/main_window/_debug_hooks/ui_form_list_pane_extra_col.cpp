#include "ui_form_list_pane_extra_col.h"
#include <QDialog>
#include <QGridLayout>
#include "widgets/DKFormListPane.h"
#include "dovah/form_stub.h"

#include "dovah/forms/Activator.h"
#include "dovah/forms/Door.h"
#include "dovah/forms/Light.h"
#include "dovah/forms/Voicetype.h"

namespace DovahKitDebug::features {
   /*static*/ void ui_form_list_pane_extra_col::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      auto* widget = new DKFormListPane(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      
      layout->addWidget(widget);

      // Test a column that requires a loaded form.
      widget->addExtraColumn("Voice Gender", [](const dovah::loaded_forms::Form& loaded) -> QString {
         if (loaded.stub.form_type != dovah::form_type::voicetype)
            return "";

         auto& casted = (dovah::loaded_forms::Voicetype&)loaded;
         if (casted.voicetype_flags & dovah::loaded_forms::Voicetype::voicetype_flag::female)
            return "Female";
         return "Male";
      });
      
      // Test a column that can work with just a stub.
      widget->addExtraColumn("Random Anim Start", [](const dovah::form_stub& stub) -> QString {
         bool flag = false;
         switch (stub.form_type) {
            case dovah::form_type::activator:
            case dovah::form_type::flora:
               flag = stub.test_record_flags(dovah::loaded_forms::Activator::form_flag::random_anim_start);
               break;
            case dovah::form_type::door:
               flag = stub.test_record_flags(dovah::loaded_forms::Door::form_flag::random_anim_start);
               break;
            case dovah::form_type::light:
               flag = stub.test_record_flags(dovah::loaded_forms::Light::form_flag::random_anim_start);
               break;
            default:
               return "";
         }

         if (flag)
            return "Yes";
         return "No";
      });
      
      dialog->show();
   }
}
