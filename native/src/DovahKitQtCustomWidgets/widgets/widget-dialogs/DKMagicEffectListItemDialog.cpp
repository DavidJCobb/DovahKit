#include "./DKMagicEffectListItemDialog.h"
#include "../DKMagicEffectListWidget.h"

#include "dovah/forms/MagicEffect.h"
#include "editor/core.h"
#include "editor/localize/magic_casting_type.h"
#include "editor/localize/magic_delivery_type.h"
#include "editor/subsystems/form_info_cache/cached_data/by_form_type/magic_effect.h"
#include "editor/subsystems/form_info_cache/core.h"

#pragma region EffectFilter
   /*virtual*/ bool DKMagicEffectListItemDialog::EffectFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
      auto& fic  = dovahkit::subsystems::form_info_cache::core::get();
      auto* info = fic.get_magic_effect_info(stub);
      if (!info)
         return true;
      if (this->_casting_type.has_value()) {
         if (info->casting_type != *this->_casting_type)
            return false;
      }
      if (this->_delivery_type.has_value()) {
         if (info->delivery_type != *this->_delivery_type)
            return false;
      }
      return true;
   }

   void DKMagicEffectListItemDialog::EffectFilter::configure(std::optional<dovah::magic_casting_type> c, std::optional<dovah::magic_delivery_type> d) {
      this->_casting_type  = c;
      this->_delivery_type = d;
      this->_refilter_all_forms();
   }
#pragma endregion

DKMagicEffectListItemDialog::DKMagicEffectListItemDialog(Context& context, QWidget* parent) : QDialog(parent), _context(context) {
   ui.setupUi(this);

   this->_cost_calculator.prepare_game_settings(this->_context.form.stub.get_owning_load_order());
   QObject::connect(&DovahKitCore::get(), &DovahKitCore::formModified, this, [this](dovah::form_stub* form) {
      if (form != this->ui.form->formStub())
         return;
      this->_pull_magic_effect_form_data(form);
      this->_update_displayed_costs();
      this->_update_displayed_duration();
   });

   //
   // Update currently displayed casting and delivery types.
   //
   {
      auto* widget = this->ui.currentCastingType;
      QString text   = tr("???");
      if (auto& opt = this->_context.casting_type; opt.has_value()) {
         auto t = editor::localize::magic_casting_type(*opt);
         if (!t.isEmpty())
            text = t;
      }
      widget->setText(text);
   }
   {
      auto*   widget = this->ui.currentDeliveryType;
      QString text   = tr("???");
      if (auto& opt = this->_context.delivery_type; opt.has_value()) {
         auto t = editor::localize::magic_delivery_type(*opt);
         if (!t.isEmpty())
            text = t;
      }
      widget->setText(text);
   }

   {
      auto* widget = this->ui.durationUnit;
      widget->clear();
      widget->addItem(tr("sec"), 1);
      widget->addItem(tr("min"), 60);
      widget->addItem(tr("hr"),  60 * 60);
      widget->addItem(tr("day"), 60 * 60 * 24);
      QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, &DKMagicEffectListItemDialog::_update_displayed_duration);
   }

   this->ui.form->setAllowedFormType(dovah::form_type::magic_effect);
   this->_effect_filter.configure(this->_context.casting_type, this->_context.delivery_type);
   this->ui.form->setCustomFilter(&this->_effect_filter);
   this->ui.form->setAllowNone(false);
   QObject::connect(this->ui.form, &DKFormPicker::formChanged, this, [this](dovah::form_stub* effect) {
      this->_pull_magic_effect_form_data(effect);
      this->_update_displayed_costs();
      this->_update_displayed_duration();
   });
   
   QObject::connect(this->ui.magnitude,    qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DKMagicEffectListItemDialog::_update_displayed_costs);
   QObject::connect(this->ui.area,         qOverload<int>(&QSpinBox::valueChanged), this, &DKMagicEffectListItemDialog::_update_displayed_costs);
   QObject::connect(this->ui.durationEdit, qOverload<int>(&QSpinBox::valueChanged), this, [this]() {
      this->_update_displayed_costs();
      this->_update_displayed_duration();
   });

   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
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
   this->ui.conditions->exportTo(this->_context.form, dst.conditions);
   return dst;
}
void DKMagicEffectListItemDialog::setData(const DKMagicEffectListModel::Item& src) {
   const auto blockers = std::array{
      QSignalBlocker(this->ui.form),
      QSignalBlocker(this->ui.area),
      QSignalBlocker(this->ui.durationUnit),
      QSignalBlocker(this->ui.durationEdit),
      QSignalBlocker(this->ui.magnitude),
      QSignalBlocker(this->ui.conditions),
   };

   this->ui.form->setFormStub(src.magic_effect);

   this->ui.area->setValue(src.area);
   this->ui.durationUnit->setCurrentIndex(0);
   this->ui.durationEdit->setValue(src.duration);
   this->ui.magnitude->setValue(src.magnitude);
   this->ui.conditions->importFrom(this->_context.form, src.conditions);

   this->_cost_calculator.effect = {
      .magnitude = src.magnitude,
      .area      = src.area,
      .duration  = src.duration,
   };
   this->_pull_magic_effect_form_data(src.magic_effect);
   this->_initial_cost = this->_cost_calculator.effect.form_info.base_cost;

   this->_update_displayed_costs();
   this->_update_displayed_duration();
}

void DKMagicEffectListItemDialog::_pull_magic_effect_form_data(dovah::form_stub* mgef) {
   auto loaded = mgef->load().ptr_cast<dovah::loaded_forms::MagicEffect>();

   this->_cost_calculator.prepare_effect_form(mgef);
   //
   this->ui.area->setDisabled(this->_cost_calculator.effect.form_info.no_area);
   this->ui.magnitude->setDisabled(this->_cost_calculator.effect.form_info.no_magnitude);
   this->ui.durationEdit->setDisabled(this->_cost_calculator.effect.form_info.no_duration);
   this->ui.durationUnit->setDisabled(this->_cost_calculator.effect.form_info.no_duration);

   this->_cached_effect_info.taper_duration = 0.0F;
   if (loaded) {
      this->_cached_effect_info.taper_duration = loaded->taper.duration;
   }
   this->ui.previewDurationTaper->setText(tr("%1 sec.").arg(this->_cached_effect_info.taper_duration));
}
void DKMagicEffectListItemDialog::_update_displayed_costs() {
   size_t effect_cost = this->_cost_calculator.calculate();
   size_t total_cost  = this->_context.spell_total_cost;
   if (!this->_context.not_yet_added) {
      total_cost -= this->_initial_cost;
   }
   total_cost += effect_cost;

   this->ui.previewEffectBaseCost->setText(QString::number(this->_cost_calculator.effect.form_info.base_cost));
   this->ui.previewEffectTotalCost->setText(QString::number(effect_cost));
   this->ui.previewSpellTotalCost->setText(QString::number(total_cost));
}
void DKMagicEffectListItemDialog::_update_displayed_duration() {
   if (this->_cost_calculator.effect.form_info.no_duration) {
      this->ui.previewFullDuration->setText(tr("N/A"));
      return;
   }

   size_t unit = this->ui.durationUnit->currentData().toInt();
   if (!unit)
      unit = 1;

   size_t value = this->ui.durationEdit->value() * unit;
   this->ui.previewFullDuration->setText(tr("%1 sec.").arg((float)value + this->_cached_effect_info.taper_duration));
}

/*virtual*/ void DKMagicEffectListItemDialog::showEvent(QShowEvent*) /*override*/ {
   //
   // Qt can be kind of stupid about dialog sizing sometimes.
   //
   if (this->_size_corrected_on_show)
      return;
   this->_size_corrected_on_show = true;
   this->resize(this->minimumSize());
};