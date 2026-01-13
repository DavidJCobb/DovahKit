#include "./scroll.h"
#include <limits>
#include <QMessageBox>
#include "editor/localize/magic_casting_type.h"
#include "editor/localize/magic_delivery_type.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"

#include "dovah/forms/MagicEffect.h"

FormDialogScroll::FormDialogScroll(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

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

   this->ui.menuDisplayObject->setAllowedFormType(dovah::form_type::statik);
   this->ui.equipType->setAllowedFormType(dovah::form_type::equip_slot);

   this->ui.range->setRange(0, 2048);
   QObject::connect(this->ui.effects, &DKMagicEffectListWidget::contentsChanged, this, [this]() {
      this->_update_auto_calc();
      this->_update_effect_parameters_enable_states();
   });
   this->ui.soundTake->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundDrop->setAllowedFormType(dovah::form_type::sound_descriptor);

   this->ui.chargeTime->setRange(0, 600);
   this->ui.cost->setRange(0, 5000);
   this->ui.castDuration->setRange(0, 600);
   QObject::connect(this->ui.flagAutoCalc, &QCheckBox::toggled, this, &FormDialogScroll::_update_auto_calc);

   QObject::connect(this->ui.buttonTestAutoDesc, &QPushButton::clicked, this, [this]() {
      QString desc = this->_make_concatenated_description();
      QMessageBox::information(this, tr("Concatenated effect description"), desc);
   });

   this->load(); // this creates the working copy.
}
void FormDialogScroll::_load_impl() {
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   {
      using enumeration = dovah::magic_casting_type;
      auto* widget = this->ui.casting;
      ui::bind(widget, working.common_data.casting_type);
      widget->setCurrentIndex(widget->findData((int)enumeration::fire_and_forget));
   }
   ui::bind(this->ui.delivery, working.common_data.delivery_type);
   ui::bind(this->ui.menuDisplayObject, working.menu_display_object, working);
   ui::bind(this->ui.equipType, working.equip_type, working);
   ui::bind(this->ui.range, working.common_data.range);
   {
      using flag  = dovah::loaded_forms::components::common_spell_data::flag;
      auto& flags = working.common_data.flags;
      ui::bind(this->ui.flagNoAbsorbReflect, flags, flag::no_absorb_reflect);
      ui::bind(this->ui.flagScriptEffectAlwaysApplies, flags, flag::script_effect_always_applies);
      ui::bind(this->ui.flagAOEIgnoresLOS, flags, flag::aoe_ignores_los);

      ui::bind_inverse(this->ui.flagAutoCalc, flags, flag::manual_cost_calc);
   }
   ui::bind(this->ui.soundTake, working.sounds.take, working);
   ui::bind(this->ui.soundDrop, working.sounds.drop, working);
   this->ui.description->setPlainText(gls.convert_localized_string(working.description));

   this->ui.destructionData->initializeFrom(working.destruction_data);
   this->ui.keywords->pullStubs(working.keywords.forms);
   this->ui.model->initializeFrom(working.model);

   this->ui.effects->setCastingType(dovah::magic_casting_type::fire_and_forget);
   this->ui.effects->setDeliveryType(working.common_data.delivery_type);
   this->ui.effects->importFrom(working, working.effects);
   ui::bind(this->ui.chargeTime, working.common_data.charge_time);
   ui::bind(this->ui.cost, working.common_data.base_cost);
   ui::bind(this->ui.castDuration, working.common_data.casting_duration);

   this->_update_auto_calc();
   this->_update_effect_parameters_enable_states();
}
void FormDialogScroll::_save_impl() {
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

   this->ui.destructionData->commitTo(working.destruction_data, working);
   this->ui.effects->exportTo(working, working.effects);
   this->ui.keywords->commitStubs(working.keywords.forms, working);
   this->ui.model->commitTo(working.model, working);
}

#include "dovah/utils/text_replacers/do_text_replacement.h"
#include "dovah/utils/text_replacers/effect_item_handler.h"
#include "widgets/widget-models/DKMagicEffectListModel.h"
QString FormDialogScroll::_make_concatenated_description() const {
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
void FormDialogScroll::_update_auto_calc() {
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
void FormDialogScroll::_update_effect_parameters_enable_states() {
   bool enable = this->ui.effects->effectCount() == 0;
   this->ui.casting->setEnabled(false);
   this->ui.delivery->setEnabled(enable);
}