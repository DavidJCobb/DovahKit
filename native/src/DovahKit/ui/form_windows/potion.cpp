#include "./potion.h"
#include <limits>
#include <QMessageBox>
#include "dovah/core.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

#include "dovah/forms/MagicEffect.h"

FormDialogPotion::FormDialogPotion(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.equipType->setAllowedFormType(dovah::form_type::equip_slot);

   this->ui.soundTake->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundDrop->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundUse->setAllowedFormType(dovah::form_type::sound_descriptor);

   this->ui.addictionSpell->setAllowedFormType(dovah::form_type::spell);

   this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });

   ui::set_range<int32_t>(this->ui.value);
   ui::set_unsigned_range<float>(this->ui.weight);

   QObject::connect(this->ui.flagAutoCalc, &QCheckBox::toggled, this, &FormDialogPotion::_update_auto_calc);
   QObject::connect(this->ui.effects, &DKMagicEffectListWidget::contentsChanged, this, &FormDialogPotion::_update_auto_calc);

   this->load(); // this creates the working copy.
}
void FormDialogPotion::_load_impl() {
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   ui::bind(this->ui.weight, working.weight);
   ui::bind(this->ui.equipType, working.equip_type, working);
   {
      using flag  = loaded_form_type::flag;
      auto& flags = working.flags;
      ui::bind(this->ui.flagFood, flags, flag::food_item);
      ui::bind(this->ui.flagPoison, flags, flag::poison);
      ui::bind(this->ui.flagMedicine, flags, flag::medicine);

      ui::bind_inverse(this->ui.flagAutoCalc, flags, flag::manual_cost_calc);
   }
   ui::bind(this->ui.soundTake, working.sounds.take, working);
   ui::bind(this->ui.soundDrop, working.sounds.drop, working);
   ui::bind(this->ui.soundUse, working.sounds.use, working);
   ui::bind(this->ui.addictionSpell, working.addiction.form, working);
   {
      auto* widget = this->ui.addictionChance;
      auto& value  = working.addiction.chance;
      widget->setValue(value * 100);
      QObject::connect(widget, qOverload<int>(&QSpinBox::valueChanged), this, [&value](int v) {
         value = (float)v / 100;
      });
   }
   ui::bind(this->ui.value, working.value);

   this->ui.destructionData->initializeFrom(working.destruction_data);
   this->ui.effects->importFrom(working, working.effects);
   this->ui.keywords->pullStubs(working.keywords.forms);
   this->ui.model->initializeFrom(working.model);

   this->_update_auto_calc();
}
void FormDialogPotion::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;
   
   gls.assign_localized_string(working.name, this->ui.name->text());
   this->ui.destructionData->commitTo(working.destruction_data, working);
   this->ui.effects->exportTo(working, working.effects);
   this->ui.keywords->commitStubs(working.keywords.forms, working);
   this->ui.model->commitTo(working.model, working);

   cobb::edit_bit(this->record_flags(), loaded_form_type::form_flag::medicine, (working.flags & loaded_form_type::flag::medicine));
}

void FormDialogPotion::_update_auto_calc() {
   bool auto_calc = this->ui.flagAutoCalc->isChecked();
   this->ui.value->setDisabled(auto_calc);
   if (!auto_calc)
      return;
   
   DKMagicEffectListWidget::AutoCalcData data;
   this->ui.effects->autoCalc(data);

   this->ui.value->setValue(data.cost);
}