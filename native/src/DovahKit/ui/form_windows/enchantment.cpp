#include "./enchantment.h"
#include <limits>
#include <QMessageBox>
#include "editor/helpers/actor_value_index_to_name.h"
#include "editor/localize/magic_casting_type.h"
#include "editor/localize/magic_delivery_type.h"
#include "editor/subsystems/form_info_cache/core.h"
#include "editor/subsystems/form_info_cache/cached_data/by_form_type/magic_effect.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "ui/utils/shrink_dialog_on_show.h"
#include "./shared/DKFormPickerExcludeSingleFormFilter.h"

#include "dovah/forms/MagicEffect.h"

FormDialogEnchantment::FormDialogEnchantment(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   ui::shrink_dialog_height_on_show(*this);

   this->_filters.exclude_self = new DKFormPickerExcludeSingleFormFilter(this);

   {
      using enumeration = decltype(loaded_form_type::enchantment_type);
      auto* widget = this->ui.type;
      widget->clear();
      widget->addItem(tr("Enchantment"), (int)enumeration::general);
      widget->addItem(tr("Staff Enchantment"), (int)enumeration::staff);
      widget->model()->sort(0);

      widget->setCurrentIndex(widget->findData((int)enumeration::general));
      QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
         auto v = (enumeration)this->ui.delivery->currentData().toInt();
         this->ui.chargeTime->setEnabled(v == enumeration::staff);
      });
   }
   {
      using enumeration = dovah::magic_casting_type;
      auto* widget = this->ui.casting;
      widget->clear();
      for (auto v : std::array{
         enumeration::concentration,
         enumeration::constant_effect,
         enumeration::fire_and_forget
      }) {
         widget->addItem(editor::localize::magic_casting_type(v), (int)v);
      }
      widget->model()->sort(0);

      widget->setCurrentIndex(widget->findData((int)enumeration::fire_and_forget));
      QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
         auto v = (enumeration)this->ui.delivery->currentData().toInt();
         this->ui.effects->setCastingType(v);
      });
   }
   {
      using enumeration = dovah::magic_delivery_type;
      auto* widget = this->ui.delivery;
      widget->clear();
      for (auto v : std::array{
         enumeration::aimed,
         enumeration::self,
         enumeration::target_actor,
         enumeration::target_location,
         enumeration::touch
      }) {
         widget->addItem(editor::localize::magic_delivery_type(v), (int)v);
      }
      widget->model()->sort(0);
      widget->setCurrentIndex(widget->findData((int)enumeration::self));

      QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
         auto v = (enumeration)this->ui.delivery->currentData().toInt();
         this->ui.effects->setDeliveryType(v);
      });
   }

   this->ui.base->setAllowedFormType(dovah::form_type::enchantment);
   this->ui.base->setCustomFilter(this->_filters.exclude_self);

   this->ui.wornRestrictions->setAllowedFormType(dovah::form_type::formlist);

   this->ui.chargeTime->setRange(0, 600);
   this->ui.cost->setRange(0, 5000);
   ui::set_range<int32_t>(this->ui.chargeAmount);

   QObject::connect(this->ui.flagAutoCalc, &QCheckBox::toggled, this, &FormDialogEnchantment::_update_auto_calc);
   QObject::connect(this->ui.effects, &DKMagicEffectListWidget::contentsChanged, this, [this]() {
      this->_update_auto_calc();
      this->_update_effect_parameters_enable_states();
   });

   QObject::connect(this->ui.flagAutoCalc, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.chargeTime->setDisabled(checked);
      this->ui.cost->setDisabled(checked);
   });

   QObject::connect(this->ui.casting, &QComboBox::currentIndexChanged, this, [this]() {
      this->ui.effects->setCastingType((dovah::magic_casting_type)this->ui.casting->currentData().toInt());
      this->_update_effect_parameters_enable_states();
   });
   QObject::connect(this->ui.delivery, &QComboBox::currentIndexChanged, this, [this]() {
      this->ui.effects->setDeliveryType((dovah::magic_delivery_type)this->ui.delivery->currentData().toInt());
      this->_update_effect_parameters_enable_states();
   });
   {
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
         if (!stub || stub->form_type != dovah::form_type::magic_effect)
            return;
         this->_update_effect_parameters_enable_states(stub);
      });
   }

   this->load(); // this creates the working copy.
}
void FormDialogEnchantment::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   this->_filters.exclude_self->set_exclusion(&working.stub);

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   ui::bind(this->ui.type, working.enchantment_type);
   {
      const auto blockers = std::array{
         QSignalBlocker(this->ui.casting),
         QSignalBlocker(this->ui.delivery),
      };
      ui::bind(this->ui.casting, working.casting_type);
      ui::bind(this->ui.delivery, working.delivery_type);
   }
   ui::bind(this->ui.base, working.base_enchantment, working);
   ui::bind(this->ui.wornRestrictions, working.worn_restrictions, working);
   {
      using flag  = loaded_form_type::flag;
      auto& flags = working.flags;
      ui::bind(this->ui.flagExtendOnRecast, flags, flag::extend_duration_on_recast);

      ui::bind_inverse(this->ui.flagAutoCalc, flags, flag::manual_cost_calc);
   }

   this->ui.effects->setCastingType(working.casting_type);
   this->ui.effects->setDeliveryType(working.delivery_type);
   this->ui.effects->importFrom(working, working.effects);
   ui::bind(this->ui.chargeTime, working.charge_time);
   ui::bind(this->ui.cost, working.cost);
   ui::bind(this->ui.chargeAmount, working.charge_amount);

   this->_update_auto_calc();
   this->_update_effect_parameters_enable_states();
}
void FormDialogEnchantment::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;
   
   gls.assign_localized_string(working.name, this->ui.name->text());

   this->ui.effects->exportTo(working, working.effects);
   this->ui.effects->disconnect();
}

void FormDialogEnchantment::_update_auto_calc() {
   bool auto_calc = this->ui.flagAutoCalc->isChecked();
   this->ui.cost->setDisabled(auto_calc);
   this->ui.chargeAmount->setDisabled(auto_calc);
   this->ui.chargeTime->setDisabled(auto_calc);
   if (!auto_calc)
      return;
   
   DKMagicEffectListWidget::AutoCalcData data;
   this->ui.effects->autoCalc(data);

   this->ui.cost->setValue(data.cost);
   this->ui.chargeAmount->setValue(data.cost);
   this->ui.chargeTime->setValue(data.charge_time);
}
void FormDialogEnchantment::_update_effect_parameters_enable_states(dovah::form_stub* changed) {
   auto effect_stubs = this->ui.effects->magicEffects();
   if (effect_stubs.empty()) {
      this->ui.casting->setEnabled(true);
      this->ui.delivery->setEnabled(true);
      return;
   }
   if (changed) {
      if (!effect_stubs.contains(changed))
         return;
   }

   bool inconsistent = false;
   {
      auto casting  = (dovah::magic_casting_type)  this->ui.casting->currentData().toInt();
      auto delivery = (dovah::magic_delivery_type) this->ui.delivery->currentData().toInt();
      for (auto* effect : effect_stubs) {
         auto loaded = effect->load().ptr_cast<dovah::loaded_forms::MagicEffect>();
         if (!loaded)
            continue;
         if (casting != loaded->casting_type || delivery != loaded->delivery_type) {
            inconsistent = true;
            break;
         }
      }
   }
   this->ui.casting->setEnabled(inconsistent);
   this->ui.delivery->setEnabled(inconsistent);
}