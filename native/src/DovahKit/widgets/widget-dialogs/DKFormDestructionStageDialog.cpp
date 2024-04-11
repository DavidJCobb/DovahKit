#include "./DKFormDestructionStageDialog.h"
#include <limits>

DKFormDestructionStageDialog::DKFormDestructionStageDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   this->ui.debris->setAllowedFormType(dovah::form_type::debris);
   this->ui.explosion->setAllowedFormType(dovah::form_type::explosion);

   // consistency with CK:
   this->ui.selfDamage->setMaximum(std::numeric_limits<int32_t>::max());
}

DKFormDestructionStageDialog::DestructionStage DKFormDestructionStageDialog::value() const {
   DestructionStage out = {
      .health_percent   = this->ui.healthPerc->value(),
      .damage_stage     = this->ui.damageStage->value(),
      .self_damage_rate = this->ui.selfDamage->value(),
      .explosion        = this->ui.explosion->formStub(),
      .debris           = this->ui.debris->formStub(),
      .debris_count     = this->ui.debrisCount->value(),
   };
   if (this->ui.flagCapDamage->isChecked())
      out.flags |= DestructionStageFlag::cap_damage;
   if (this->ui.flagDestroyObject->isChecked())
      out.flags |= DestructionStageFlag::destroy_object;
   if (this->ui.flagDisableObject->isChecked())
      out.flags |= DestructionStageFlag::disable_object;
   if (this->ui.flagIgnoreExternal->isChecked())
      out.flags |= DestructionStageFlag::ignore_external_damage;
   out.replacement_model = this->ui.model->value();
   return out;
}
void DKFormDestructionStageDialog::setValue(const DestructionStage& src) {
   this->ui.healthPerc->setValue(src.health_percent);
   this->ui.damageStage->setValue(src.damage_stage);
   this->ui.selfDamage->setValue(src.self_damage_rate);
   this->ui.explosion->setFormStub(src.explosion);
   this->ui.debris->setFormStub(src.debris);
   this->ui.debrisCount->setValue(src.debris_count);
   this->ui.model->setValue(src.replacement_model);

   this->ui.flagCapDamage->setChecked(src.flags & DestructionStageFlag::cap_damage);
   this->ui.flagDestroyObject->setChecked(src.flags & DestructionStageFlag::destroy_object);
   this->ui.flagDisableObject->setChecked(src.flags & DestructionStageFlag::disable_object);
   this->ui.flagIgnoreExternal->setChecked(src.flags & DestructionStageFlag::ignore_external_damage);
}