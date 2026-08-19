#include "./FormSubdialogPerkEntry.h"
#include "dovah/data/actor_values.h"
#include "dovah/forms/Perk.h"
#include "editor/localize/entry_point_function.h"
#include "editor/localize/perk_entry_point.h"
#include "editor/localize/perk_entry_point_explanation.h"
#include "ui/utils/enum_dropdown_configs/actor_value_index.h"
#include "ui/utils/set_range.h"
#include "widgets/DKConditionList.h"

namespace {
   enum class entry_type {
      quest,
      spell,
      entry_point,
   };
}

FormSubdialogPerkEntry::FormSubdialogPerkEntry(dovah::loaded_forms::Perk& perk, QWidget* parent) : QDialog(parent), _state({ .form = perk }) {
   this->ui.setupUi(this);
   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   this->setWindowFlags(this->windowFlags() | Qt::WindowContextHelpButtonHint); // show "What's This?" button in title bar

   this->ui.rank->setRange(0, perk.data.rank_count);
   {
      auto* widget = this->ui.type;
      widget->clear();
      widget->addItem(tr("Quest"), (int)entry_type::quest);
      widget->addItem(tr("Ability"), (int)entry_type::spell);
      widget->addItem(tr("Entry Point"), (int)entry_type::entry_point);
   }
   
   this->ui.questForm->setAllowedFormType(dovah::form_type::quest);
   ui::set_range<uint16_t>(this->ui.questStage);

   this->ui.abilityForm->setAllowedFormType(dovah::form_type::spell);

   ui::enum_dropdown_configs::actor_value_index<ui::enum_dropdown_configs::actor_value_index_options{
      .allow_none = false,
      .sorted     = true,
   }>(this->ui.entryPointParamsOneAVOneFloat_AV);
   this->ui.entryPointParamsActivateChoice_Spell->setAllowedFormType(dovah::form_type::spell);

   QObject::connect(this->ui.entryPointType, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
      auto* widget = this->ui.entryPointType;
      auto  entry  = (dovah::perk_entry_point)widget->currentData().toInt();
      if ((size_t)entry >= dovah::all_perk_entry_points.size()) {
         this->_rebuild_entry_point_condition_tabs(nullptr);
         this->ui.entryPointExplanation->setText("");
         this->ui.entryPointExplanation->setWhatsThis("");
         return;
      }
      const auto& info = dovah::all_perk_entry_points[(size_t)entry];
      this->_rebuild_entry_point_function_type_combobox(info.value_type);
      this->_rebuild_entry_point_condition_tabs(&info);
      this->ui.entryPointExplanation->setText(editor::localize::perk_entry_point_explanation(entry));
      this->ui.entryPointExplanation->setWhatsThis(editor::localize::elaborated::perk_entry_point_explanation(entry));
   });
   QObject::connect(this->ui.entryPointFunction, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
      this->_update_entry_point_arguments((dovah::entry_point_function)this->ui.entryPointFunction->currentData().toInt());
   });

   ui::set_range<float>(this->ui.entryPointParamsOneFloat_Value);
   ui::set_range<float>(this->ui.entryPointParamsTwoFloats_ValueA);
   ui::set_range<float>(this->ui.entryPointParamsTwoFloats_ValueB);
   ui::set_range<float>(this->ui.entryPointParamsOneAVOneFloat_Float);
   
   //
   // Now that the event handlers are in place, set up the entry point combobox. This'll trigger signals that 
   // call the various other "update" functions.
   //
   {
      auto* widget = this->ui.entryPointType;
      widget->clear();
      for (auto& info : dovah::all_perk_entry_points) {
         auto name = editor::localize::perk_entry_point(info.id);
         widget->addItem(name, (int)info.id);
      }
      widget->model()->sort(0);
   }

   this->_update_options();
   QObject::connect(this->ui.type, qOverload<int>(&QComboBox::currentIndexChanged), this, &FormSubdialogPerkEntry::_update_options);
}

void FormSubdialogPerkEntry::setScriptListWidget(DKPapyrusBoundScriptListPane* widget) {
   this->ui.entryPointParamsActivateChoice_Fragment->setSourceWidget(widget);
}

FormSubdialogPerkEntry::value_type FormSubdialogPerkEntry::value() const {
   value_type dst;
   dst.priority = this->ui.priority->value();
   dst.rank     = this->ui.rank->value();
   switch ((entry_type)this->ui.type->currentIndex()) {
      case entry_type::quest:
         {
            auto& dst_casted = dst.data.emplace<ui::types::perk_entries::quest_entry>();
            dst_casted.quest = this->ui.questForm->formStub();
            dst_casted.stage = this->ui.questStage->value();
         }
         break;
      case entry_type::spell:
         {
            auto& dst_casted = dst.data.emplace<ui::types::perk_entries::spell_entry>();
            dst_casted.spell = this->ui.abilityForm->formStub();
         }
         break;
      case entry_type::entry_point:
         {
            auto& dst_casted = dst.data.emplace<ui::types::perk_entries::entry_point_entry>();
            dst_casted.entry_point = (dovah::perk_entry_point)this->ui.entryPointType->currentData().toInt();
            dst_casted.set_function((dovah::entry_point_function)this->ui.entryPointFunction->currentData().toInt());
            switch (dovah::entry_point_value_type_of(dst_casted.function)) {
               case dovah::entry_point_value_type::activate_choice:
                  {
                     auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::activate_choice>();
                     params.label = this->ui.entryPointParamsActivateChoice_Text->text();
                     params.spell = this->ui.entryPointParamsActivateChoice_Spell->formStub();
                     params.replace_default = this->ui.entryPointParamsActivateChoice_ReplaceDefault->isChecked();
                     params.run_immediately = this->ui.entryPointParamsActivateChoice_RunImmediately->isChecked();
                     {
                        auto* src = this->ui.entryPointParamsActivateChoice_Fragment;
                        auto& dst = params.fragment;
                        dst.script   = src->currentScriptname();
                        dst.function = src->currentFunction();
                     }
                  }
                  break;
               case dovah::entry_point_value_type::leveled_item:
               case dovah::entry_point_value_type::spell:
                  {
                     auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::form>();
                     params.value = this->ui.entryPointParamsForm_Form->formStub();
                  }
                  break;
               case dovah::entry_point_value_type::localized_string:
                  {
                     auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::localized_string>();
                     params.value = this->ui.entryPointParamsLocalizedString_Text->text();
                  }
                  break;
               case dovah::entry_point_value_type::none:
                  dst_casted.parameters.emplace<std::monostate>();
                  break;
               case dovah::entry_point_value_type::number:
                  {
                     auto* container = this->ui.entryPointParamsStack->currentWidget();
                     if (container == this->ui.entryPointParamsOneAVOneFloat) {
                        auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::one_av_one_float>();
                        params.actor_value = this->ui.entryPointParamsOneAVOneFloat_AV->currentData().toInt();
                        params.value       = this->ui.entryPointParamsOneAVOneFloat_Float->value();
                     } else if (container == this->ui.entryPointParamsOneFloat) {
                        auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::one_float>();
                        params.value = this->ui.entryPointParamsOneFloat_Value->value();
                     } else if (container == this->ui.entryPointParamsTwoFloats) {
                        auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::two_floats>();
                        params.values[0] = this->ui.entryPointParamsTwoFloats_ValueA->value();
                        params.values[1] = this->ui.entryPointParamsTwoFloats_ValueB->value();
                     }
                  }
                  break;
               case dovah::entry_point_value_type::raw_string:
                  {
                     auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::raw_string>();
                     params.value = this->ui.entryPointParamsRawString_Text->text();
                  }
                  break;
            }

            auto& cnd_group_widgets = this->_state.entry_point_condition_groups;
            const size_t size = cnd_group_widgets.size();
            dst_casted.conditions_by_entity.resize(size);
            for (size_t i = 0; i < size; ++i) {
               auto* widget = cnd_group_widgets[i].get();
               if (!widget)
                  break;
               widget->exportTo(this->_state.form, dst_casted.conditions_by_entity[i]);
            }
         }
         break;
   }
   return dst;
}
void FormSubdialogPerkEntry::setValue(const value_type& src) {
   this->ui.rank->setValue(src.rank);
   this->ui.priority->setValue(src.priority);
   if (auto* casted = std::get_if<ui::types::perk_entries::quest_entry>(&src.data)) {
      this->ui.type->setCurrentIndex(this->ui.type->findData((int)entry_type::quest));
      this->ui.questForm->setFormStub(casted->quest);
      this->ui.questStage->setValue(casted->stage);
   } else if (auto* casted = std::get_if<ui::types::perk_entries::spell_entry>(&src.data)) {
      this->ui.type->setCurrentIndex(this->ui.type->findData((int)entry_type::spell));
      this->ui.abilityForm->setFormStub(casted->spell);
   } else if (auto* casted = std::get_if<ui::types::perk_entries::entry_point_entry>(&src.data)) {
      this->ui.type->setCurrentIndex(this->ui.type->findData((int)entry_type::entry_point));
      {
         auto* widget = this->ui.entryPointType;
         auto  i      = widget->findData((int)casted->entry_point);
         widget->setCurrentIndex(i);
      }
      {
         auto* widget = this->ui.entryPointFunction;
         auto  i      = widget->findData((int)casted->function);
         widget->setCurrentIndex(i);
      }
      if (auto* params = std::get_if<ui::types::perk_entries::params::activate_choice>(&casted->parameters)) {
         this->ui.entryPointParamsActivateChoice_Text->setText(params->label);
         this->ui.entryPointParamsActivateChoice_Spell->setFormStub(params->spell);
         this->ui.entryPointParamsActivateChoice_ReplaceDefault->setChecked(params->replace_default);
         this->ui.entryPointParamsActivateChoice_RunImmediately->setChecked(params->run_immediately);
         {
            auto* widget = this->ui.entryPointParamsActivateChoice_Fragment;
            widget->setCurrentScriptname(params->fragment.script);
            widget->setCurrentFunction(params->fragment.function);
         }
      } else if (auto* params = std::get_if<ui::types::perk_entries::params::form>(&casted->parameters)) {

         this->ui.entryPointParamsForm_Form->setFormStub(params->value);
      } else if (auto* params = std::get_if<ui::types::perk_entries::params::localized_string>(&casted->parameters)) {
         this->ui.entryPointParamsLocalizedString_Text->setText(params->value);
      } else if (auto* params = std::get_if<ui::types::perk_entries::params::one_av_one_float>(&casted->parameters)) {
         {
            auto* widget = this->ui.entryPointParamsOneAVOneFloat_AV;
            widget->setCurrentIndex(widget->findData(params->actor_value));
         }
         this->ui.entryPointParamsOneAVOneFloat_Float->setValue(params->value);
      } else if (auto* params = std::get_if<ui::types::perk_entries::params::one_float>(&casted->parameters)) {
         this->ui.entryPointParamsOneFloat_Value->setValue(params->value);
      } else if (auto* params = std::get_if<ui::types::perk_entries::params::raw_string>(&casted->parameters)) {
         this->ui.entryPointParamsRawString_Text->setText(params->value);
      } else if (auto* params = std::get_if<ui::types::perk_entries::params::two_floats>(&casted->parameters)) {
         this->ui.entryPointParamsTwoFloats_ValueA->setValue(params->values[0]);
         this->ui.entryPointParamsTwoFloats_ValueB->setValue(params->values[1]);
      }

      const size_t size = casted->conditions_by_entity.size();
      for (size_t i = 0; i < size; ++i) {
         auto* widget = this->_state.entry_point_condition_groups[i].get();
         if (!widget)
            break;
         widget->importFrom(this->_state.form, casted->conditions_by_entity[i]);
      }
   }
}

void FormSubdialogPerkEntry::_update_options() {
   switch ((entry_type)this->ui.type->currentIndex()) {
      case entry_type::quest:
         this->ui.optionsStack->setCurrentWidget(this->ui.optionsQuest);
         break;
      case entry_type::spell:
         this->ui.optionsStack->setCurrentWidget(this->ui.optionsAbility);
         break;
      case entry_type::entry_point:
         this->ui.optionsStack->setCurrentWidget(this->ui.optionsEntryPoint);
         break;
   }
}
void FormSubdialogPerkEntry::_rebuild_entry_point_condition_tabs(const dovah::perk_entry_point_info* info) {
   auto& widget_list = this->_state.entry_point_condition_groups;

   auto* tabview = this->ui.entryPointConditionsTabbox;
   size_t size = tabview->count();
   for (size_t i = 0; i < size; ++i) {
      auto* tab = tabview->widget(0);
      tabview->removeTab(0);
      tab->deleteLater();
   }
   widget_list.clear();

   size = 0;
   if (info) {
      size = info->arg_count();
      for (size_t i = 0; i < size; ++i) {
         auto& subject = info->args[i];

         // DKConditionList needs a "context" so it can actually allow editing of conditions. 
         // If we're editing a Perk Entry that already exists, the context is set when we 
         // import its data. However, if we're creating a new Perk Entry (such that this 
         // dialog isn't pulling from something that already exists), we have to tell the 
         // DKConditionList what form "owns" the conditions, so it can compute the context 
         // from that information.
         auto* cnd_list = new DKConditionList(this);
         cnd_list->overrideOwningForm(this->_state.form);
         widget_list.push_back(cnd_list);

         QString explanatory;
         switch (subject.type) {
            case dovah::form_type::enchantment:
               explanatory = tr(
                  "<p>The Subject is the Enchantment: the game pretends to be working with a ref whose "
                  "base form is the Enchantment, so conditions that operate on the base form, like "
                  "<code>GetIsID</code>, will see the Enchantment.</p>"
               );
               break;
            case dovah::form_type::spell:
               explanatory = tr(
                  "<p>The Subject is the Spell: the game pretends to be working with a ref whose base "
                  "form is the Spell, so conditions that operate on the base form, like "
                  "<code>GetIsID</code>, will see the Enchantment. There also exist conditions "
                  "specifically made for entry points like this, with names beginning with "
                  "<code>EPMagic_</code>.</p>"
               );
               break;
         }
         auto* wrap   = new QWidget(this);
         auto* layout = new QVBoxLayout(wrap);
         layout->addWidget(cnd_list, 1);
         if (!explanatory.isEmpty()) {
            auto* text = new QLabel(this);
            text->setWordWrap(true);
            text->setText(explanatory);
            layout->addWidget(text);
         }
         tabview->addTab(wrap, QString(subject.name));
      }
   }
   if (!size) {
      auto* tab = new DKConditionList(this);
      tab->setEnabled(false);
      tab->overrideOwningForm(this->_state.form);
      tabview->addTab(tab, "Perk Owner");
      tabview->setTabEnabled(0, false);
      widget_list.push_back(tab);
   }
}
void FormSubdialogPerkEntry::_rebuild_entry_point_function_type_combobox(dovah::entry_point_value_type vt) {
   using value_type = dovah::entry_point_function;
   constexpr const auto list = std::array{
      value_type::none,
      value_type::set_value,
      value_type::add_value,
      value_type::multiply_value,
      value_type::add_range_to_value,
      value_type::add_actor_value_mult,
      value_type::absolute_value,
      value_type::negative_absolute_value,
      value_type::add_leveled_list,
      value_type::add_activate_choice,
      value_type::select_spell,
      value_type::select_text,
      value_type::set_to_actor_value_mult,
      value_type::multiply_actor_value_mult,
      value_type::multiply_one_plus_av_mult,
      value_type::set_text,
   };

   auto*      widget  = this->ui.entryPointFunction;
   auto       prior   = (value_type)widget->currentData().toInt();
   const auto blocker = QSignalBlocker(widget);
   widget->clear();
   for (auto v : list) {
      auto pair_type = dovah::entry_point_value_type_of(v);
      if (pair_type != vt)
         continue;
      auto name = editor::localize::entry_point_function(v);
      widget->addItem(name, (int)v);
   }
   auto i = widget->findData((int)prior);
   if (i >= 0)
      widget->setCurrentIndex(i);

   auto after = (value_type)widget->currentData().toInt();
   this->_update_entry_point_arguments(after);
}
void FormSubdialogPerkEntry::_update_entry_point_arguments(dovah::entry_point_function func) {
   auto  type  = dovah::expected_type_for_entry_point_function(func);
   auto* stack = this->ui.entryPointParamsStack;
   switch (type) {
      case dovah::entry_point_function_type::activate_choice:
         stack->setCurrentWidget(this->ui.entryPointParamsActivateChoice);
         break;
      case dovah::entry_point_function_type::animation_graph_var:
         this->ui.entryPointParamsRawString_Label->setText(tr("Variable Name:"));
         stack->setCurrentWidget(this->ui.entryPointParamsRawString);
         break;
      case dovah::entry_point_function_type::leveled_item:
         this->ui.entryPointParamsForm_Desc->setText(tr("Leveled Item:"));
         this->ui.entryPointParamsForm_Form->setAllowedFormType(dovah::form_type::leveled_item);
         stack->setCurrentWidget(this->ui.entryPointParamsForm);
         break;
      case dovah::entry_point_function_type::localized_string:
         stack->setCurrentWidget(this->ui.entryPointParamsLocalizedString);
         break;
      case dovah::entry_point_function_type::spell:
         this->ui.entryPointParamsForm_Desc->setText(tr("Spell:"));
         this->ui.entryPointParamsForm_Form->setAllowedFormType(dovah::form_type::spell);
         stack->setCurrentWidget(this->ui.entryPointParamsForm);
         break;
      case dovah::entry_point_function_type::one_float:
         stack->setCurrentWidget(this->ui.entryPointParamsOneFloat);
         {
            auto* label = this->ui.entryPointParamsOneFloat_Prefix;
            switch (func) {
               case dovah::entry_point_function::add_value:
                  label->setText(tr("Value = Value + "));
                  break;
               case dovah::entry_point_function::multiply_value:
                  label->setText(tr("Value = Value * "));
                  break;
               case dovah::entry_point_function::set_value:
                  label->setText(tr("Value = "));
                  break;
            }
         }
         break;
      case dovah::entry_point_function_type::two_floats:
         if (dovah::entry_point_function_takes_an_av(func)) {
            stack->setCurrentWidget(this->ui.entryPointParamsOneAVOneFloat);
            auto* label_a = this->ui.entryPointParamsOneAVOneFloat_Prefix;
            auto* label_b = this->ui.entryPointParamsOneAVOneFloat_Middle;
            auto* label_c = this->ui.entryPointParamsOneAVOneFloat_Suffix;
            switch (func) {
               case dovah::entry_point_function::add_actor_value_mult:
                  label_a->setText(tr("Value = Value + ("));
                  label_b->setText(tr(" * "));
                  label_c->setText(tr(")"));
                  break;
               case dovah::entry_point_function::multiply_actor_value_mult:
                  label_a->setText(tr("Value = Value * "));
                  label_b->setText(tr(" * "));
                  label_c->setText(tr(""));
                  break;
               case dovah::entry_point_function::multiply_one_plus_av_mult:
                  label_a->setText(tr("Value = Value * (1 + ("));
                  label_b->setText(tr(" * "));
                  label_c->setText(tr("))"));
                  break;
               case dovah::entry_point_function::set_to_actor_value_mult:
                  label_a->setText(tr("Value = "));
                  label_b->setText(tr(" * "));
                  label_c->setText(tr(""));
                  break;
            }
         } else {
            stack->setCurrentWidget(this->ui.entryPointParamsTwoFloats);
            auto* label_a = this->ui.entryPointParamsTwoFloats_Prefix;
            auto* label_b = this->ui.entryPointParamsTwoFloats_Middle;
            auto* label_c = this->ui.entryPointParamsTwoFloats_Suffix;
            switch (func) {
               case dovah::entry_point_function::add_range_to_value:
                  label_a->setText(tr("Value = Value + rand("));
                  label_b->setText(tr(", "));
                  label_c->setText(tr(")"));
                  break;
            }
         }
         break;
      case dovah::entry_point_function_type::none:
         stack->setCurrentWidget(this->ui.entryPointParamsNone);
         {
            auto* label = this->ui.entryPointParamsNone_Desc;
            switch (func) {
               case dovah::entry_point_function::none:
                  label->setText(tr(""));
                  break;
               case dovah::entry_point_function::absolute_value:
                  label->setText(tr("Value = abs(Value)"));
                  break;
               case dovah::entry_point_function::negative_absolute_value:
                  label->setText(tr("Value = -abs(Value)"));
                  break;
            }
         }
         break;
   }

}