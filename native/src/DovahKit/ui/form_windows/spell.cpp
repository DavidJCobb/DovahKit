#include "./spell.h"
#include <limits>
#include <QMessageBox>
#include "editor/helpers/actor_value_index_to_name.h"
#include "editor/localize/magic_casting_type.h"
#include "editor/localize/magic_delivery_type.h"
#include "editor/subsystems/form_info_cache/core.h"
#include "editor/subsystems/form_info_cache/cached_data/by_form_type/magic_effect.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"

#include "dovah/forms/MagicEffect.h"

FormDialogSpell::FormDialogSpell(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   {
      using enumeration = decltype(dovah::loaded_forms::components::common_spell_data::type);
      auto* widget = this->ui.type;
      widget->clear();
      widget->addItem(tr("Ability"), (int)enumeration::ability);
      widget->addItem(tr("Addiction"), (int)enumeration::addiction);
      widget->addItem(tr("Disease"), (int)enumeration::disease);
      widget->addItem(tr("Lesser Power"), (int)enumeration::lesser_power);
      widget->addItem(tr("Poison"), (int)enumeration::poison);
      widget->addItem(tr("Power"), (int)enumeration::power);
      widget->addItem(tr("Spell"), (int)enumeration::spell);
      widget->addItem(tr("Voice Power"), (int)enumeration::voice);
      widget->model()->sort(0);
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

      QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, &FormDialogSpell::_update_condition_explanation);

      widget->setCurrentIndex(widget->findData((int)enumeration::fire_and_forget));
      QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
         auto v = (enumeration)this->ui.delivery->currentData().toInt();
         this->ui.castDuration->setEnabled(v != enumeration::concentration);

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
         if (v == enumeration::target_actor || v == enumeration::target_location) {
            this->ui.range->setEnabled(true);
         } else {
            this->ui.range->setEnabled(false);
         }

         this->ui.effects->setDeliveryType(v);
      });
   }

   this->ui.menuDisplay->setAllowedFormType(dovah::form_type::statik);
   this->ui.equipSlot->setAllowedFormType(dovah::form_type::equip_slot);
   this->ui.castingPerk->setAllowedFormType(dovah::form_type::perk);

   this->ui.range->setRange(0, 2048);
   this->ui.school->setReadOnly(true);
   QObject::connect(this->ui.effects, &DKMagicEffectListWidget::contentsChanged, this, [this]() {
      auto*   effect = this->ui.effects->costliestMagicEffect();
      QString text   = tr("NONE");
      if (effect) {
         auto loaded = effect->load().ptr_cast<dovah::loaded_forms::MagicEffect>();
         if (loaded) {
            text = editor_helpers::actor_value_index_to_name(loaded->magic_skill);
         } else {
            text = tr("???");
         }
      }
      this->ui.school->setText(text);

      this->_update_auto_calc();
      this->_update_effect_parameters_enable_states();
   });

   this->ui.chargeTime->setRange(0, 600);
   this->ui.cost->setRange(0, 5000);
   this->ui.castDuration->setRange(0, 600);
   QObject::connect(this->ui.flagAutoCalc, &QCheckBox::toggled, this, &FormDialogSpell::_update_auto_calc);

   QObject::connect(this->ui.buttonTestAutoDesc, &QPushButton::clicked, this, [this]() {
      QString desc = this->_make_concatenated_description();
      QMessageBox::information(this, tr("Concatenated effect description"), desc);
   });

   this->load(); // this creates the working copy.
}
void FormDialogSpell::_load_impl() {
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   ui::bind(this->ui.type, working.common_data.type);
   ui::bind(this->ui.casting, working.common_data.casting_type);
   ui::bind(this->ui.delivery, working.common_data.delivery_type);
   ui::bind(this->ui.menuDisplay, working.menu_display_object, working);
   ui::bind(this->ui.equipSlot, working.equip_type, working);
   ui::bind(this->ui.castingPerk, working.common_data.half_cost_perk, working);
   ui::bind(this->ui.range, working.common_data.range);
   {
      using flag  = dovah::loaded_forms::components::common_spell_data::flag;
      auto& flags = working.common_data.flags;
      ui::bind(this->ui.flagNoAbsorbReflect,  flags, flag::no_absorb_reflect);
      ui::bind(this->ui.flagIgnoreResist,     flags, flag::ignore_resistance);
      ui::bind(this->ui.flagAOEIgnoresLOS,    flags, flag::aoe_ignores_los);
      ui::bind(this->ui.flagPlayerStartSpell, flags, flag::pc_start_spell);
      ui::bind(this->ui.flagNoDualCastMod,    flags, flag::no_dual_cast_mod);

      ui::bind_inverse(this->ui.flagAutoCalc, flags, flag::manual_cost_calc);
   }
   this->ui.description->setPlainText(gls.convert_localized_string(working.description));

   this->ui.effects->setCastingType(working.common_data.casting_type);
   this->ui.effects->setDeliveryType(working.common_data.delivery_type);
   this->ui.effects->importFrom(working, working.effects);
   ui::bind(this->ui.chargeTime, working.common_data.charge_time);
   ui::bind(this->ui.cost, working.common_data.base_cost);
   ui::bind(this->ui.castDuration, working.common_data.casting_duration);

   this->_update_auto_calc();
   this->_update_effect_parameters_enable_states();
}
void FormDialogSpell::_save_impl() {
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
   gls.assign_localized_string(working.description, this->ui.description->toPlainText());

   this->ui.effects->exportTo(working, working.effects);
}

#include "dovah/utils/text_replacers/do_text_replacement.h"
#include "dovah/utils/text_replacers/effect_item_handler.h"
#include "widgets/widget-models/DKMagicEffectListModel.h"
QString FormDialogSpell::_make_concatenated_description() const {
   std::string text;
   size_t      size = this->ui.effects->effectCount();
   for (size_t i = 0; i < size; ++i) {
      const auto* data = this->ui.effects->effect(i);
      if (!data)
         continue;

      dovah::text_replacers::effect_item_handler handler;
      handler.magic_effect = data->magic_effect;
      handler.area         = data->area;
      handler.duration     = data->duration;
      handler.magnitude    = data->magnitude;

      if (i)
         text += ' ';
      auto loaded = data->magic_effect->load().ptr_cast<dovah::loaded_forms::MagicEffect>();
      if (loaded) {
         text += dovah::text_replacers::do_text_replacement(loaded->description.c_str(), handler);
      } else {
         text += dovah::text_replacers::do_text_replacement("", handler);
      }
   }
   return QString::fromStdString(text);
}
void FormDialogSpell::_update_auto_calc() {
   bool auto_calc = this->ui.flagAutoCalc->isChecked();
   this->ui.cost->setDisabled(auto_calc);
   this->ui.chargeTime->setDisabled(auto_calc);
   if (!auto_calc)
      return;

   DKMagicEffectListWidget::AutoCalcData data;
   this->ui.effects->autoCalc(data);

   this->ui.cost->setValue(data.cost);
   this->ui.chargeTime->setValue(data.charge_time);
}
void FormDialogSpell::_update_condition_explanation() {
   auto v = (dovah::magic_casting_type)this->ui.casting->currentData().toInt();
   if (v == dovah::magic_casting_type::concentration) {
      this->ui.conditionExplanationStack->setCurrentWidget(this->ui.conditionExplanationConcentration);
   } else {
      this->ui.conditionExplanationStack->setCurrentWidget(this->ui.conditionExplanationNormal);
   }
}
void FormDialogSpell::_update_effect_parameters_enable_states() {
   bool enable = this->ui.effects->effectCount() == 0;
   this->ui.casting->setEnabled(enable);
   this->ui.delivery->setEnabled(enable);
}