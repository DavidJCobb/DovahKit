#include "./DKMagicEffectListItemDialog.h"
#include "../DKMagicEffectListWidget.h"

DKMagicEffectListItemDialog::DKMagicEffectListItemDialog(DKMagicEffectListWidget& parent, dovah::loaded_forms::Form& form) : QDialog(&parent), _owner(parent), _form(form) {
   ui.setupUi(this);

   static_assert(false, "TODO: Update displayed casting type and delivery type");
   static_assert(false, "TODO: Cache spell total cost and level, for display");

   {
      auto* widget = this->ui.durationUnit;
      widget->clear();
      widget->addItem(tr("sec"), 1);
      widget->addItem(tr("min"), 60);
      widget->addItem(tr("hr"),  60 * 60);
      widget->addItem(tr("day"), 60 * 60 * 24);
      static_assert(false, "TODO: When unit changes, update displayed total duration");
   }

   this->ui.form->setAllowedFormType(dovah::form_type::magic_effect);
   static_assert(false, "TODO: Formpicker filter (casting type and delivery type of owning widget)");
   QObject::connect(this->ui.form, &DKFormPicker::formChanged, this, [this](dovah::form_stub* effect) {
      static_assert(false, "TODO: Update displayed durations");
      static_assert(false, "TODO: Update displayed effect base cost");
      static_assert(false, "TODO: Update displayed effect total cost");
      static_assert(false, "TODO: Update displayed spell total cost");
      static_assert(false, "TODO: Update displayed spell level");
      static_assert(false, "TODO: Update enable state for Magnitude");
      static_assert(false, "TODO: Update enable state for Area");
      static_assert(false, "TODO: Update enable state for Duration");
   });
}

[[nodiscard]] DKMagicEffectListModel::Item DKMagicEffectListItemDialog::data() const {
   DKMagicEffectListModel::Item dst;
   dst.magic_effect = this->ui.form->formStub();
   dst.area = this->ui.area->value();
   dst.magnitude = this->ui.magnitude->value();
   {
      auto unit = this->ui.durationUnit->currentData().toInt();
      if (unit <= 0)
         unit = 1;
      dst.duration = this->ui.durationEdit->value() * unit;
   }
   this->ui.conditions->exportTo(this->_form, dst.conditions);
   return dst;
}
void DKMagicEffectListItemDialog::setData(const DKMagicEffectListModel::Item& src);