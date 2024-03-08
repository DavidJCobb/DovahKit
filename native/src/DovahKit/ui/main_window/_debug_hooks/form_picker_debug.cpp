#include "form_picker_debug.h"
#include "widgets/DKFormPicker.h"
#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>

namespace DovahKitDebug::features {
   /*static*/ void debug_form_picker::execute(QWidget* parent) {
      auto* dialog = new QDialog(parent);
      auto* layout = new QGridLayout(dialog);
      dialog->setLayout(layout);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      {
         auto* picker = new DKFormPicker(dialog);
         picker->setAllowedFormTypes({ // GetIsID types
            dovah::form_type::acoustic_space,
            dovah::form_type::activator,
            dovah::form_type::actor_base,
            dovah::form_type::container,
            dovah::form_type::door,
            dovah::form_type::flora,
            dovah::form_type::furniture,
            dovah::form_type::grass,
            dovah::form_type::hazard,
            dovah::form_type::idle_marker,
            dovah::form_type::light,
            dovah::form_type::movable_static,
            dovah::form_type::projectile,
            dovah::form_type::sound,
            dovah::form_type::statik,
            dovah::form_type::talking_activator,
            dovah::form_type::tree,
            // Items:
            dovah::form_type::ammo,
            dovah::form_type::armor,
            dovah::form_type::armor_addon,
            dovah::form_type::book,
            dovah::form_type::key,
            dovah::form_type::leveled_item,
            dovah::form_type::misc_item,
            dovah::form_type::potion,
            dovah::form_type::scroll,
            dovah::form_type::soul_gem,
            dovah::form_type::weapon,
            // Magic:
            dovah::form_type::enchantment,
            dovah::form_type::leveled_spell,
            dovah::form_type::shout,
            dovah::form_type::spell,
            // Other:
            dovah::form_type::formlist,
         });
         layout->addWidget(picker, 0, 0);
         //
         auto* button = new QCheckBox("Can split by type");
         button->setChecked(true);
         QObject::connect(button, &QCheckBox::stateChanged, picker, [picker](int state) {
            picker->setSplitTypesWhenMany(state == Qt::CheckState::Checked);
         });
         layout->addWidget(button, 0, 1);
      }
      {
         auto* picker = new DKFormPicker(dialog);
         picker->setAllowedFormTypes({ dovah::form_type::cell });
         picker->setAllowNone(false);
         layout->addWidget(picker, 1, 0, 1, 2);
      }
      //
      dialog->show();
   }
}