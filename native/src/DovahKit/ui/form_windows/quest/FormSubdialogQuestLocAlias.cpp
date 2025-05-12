#include "./FormSubdialogQuestLocAlias.h"
#include "dovah/data/story_manager.h"
#include "dovah/forms/components/papyrus/attached_script.h"
#include "dovah/forms/Quest.h"
#include "./alias_subdialog_helpers.h"

namespace {
   constexpr const auto no_alias = FormSubdialogQuestLocAlias::loaded_alias_type::none_id;
   namespace alias_fill_params {
      using namespace dovah::loaded_forms::structs::alias_fill_params;
   }
   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
   }
}

FormSubdialogQuestLocAlias::FormSubdialogQuestLocAlias(loaded_form_type& quest, loaded_alias_type& alias, QWidget* parent)
   :
   QDialog(parent),
   _data{ quest, alias }
{
   this->ui.setupUi(this);

   this->ui.fillFromPredefined->setAllowedFormType(dovah::form_type::location);
   this->ui.fillFromSiblingAliasParentKeyword->setAllowedFormType(dovah::form_type::keyword);
   this->ui.fillFromExtAliasQuest->setAllowedFormType(dovah::form_type::quest);

   make_location_alias_combobox(quest, *this->ui.forceInto);
   make_reference_alias_combobox(quest, *this->ui.fillFromSiblingAlias);
   make_event_data_comboboxes(quest, *this->ui.findMatchingEventName, *this->ui.findMatchingEventData);

   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, [this]() {
      this->save();
      this->accept();
   });
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

   this->load();
}

void FormSubdialogQuestLocAlias::load() {
   this->ui.name->setText(QString::fromStdString(this->_data.alias.name));
   {
      const auto flags = this->_data.alias.flags;
      this->ui.flagReserve->setChecked(flags & loaded_alias_type::flag::reserves_target);
      this->ui.flagOptional->setChecked(flags & loaded_alias_type::flag::optional);
      this->ui.flagDisplaysText->setChecked(flags & loaded_alias_type::flag::uses_stored_text);
      this->ui.flagAllowReuse->setChecked(flags & loaded_alias_type::flag::allow_reuse_in_quest);
      this->ui.flagAllowReserved->setChecked(flags & loaded_alias_type::flag::allow_reserved);
      this->ui.flagAllowCleared->setChecked(flags & loaded_alias_type::flag::allow_cleared);
   }
   {
      auto* widget   = this->ui.forceInto;
      auto  alias_id = this->_data.alias.force_into_alias_id;
      set_combobox_to_alias(*widget, alias_id);
   }
   {  // Fill type
      auto& fill = this->_data.alias.fill_params;
      if (auto* data = std::get_if<alias_fill_params::loc::preassigned>(&fill)) {
         this->ui.fillTypePredefined->setChecked(true);
         this->ui.fillFromPredefined->setFormStub(data->location.get_form_stub());
      } else if (auto* data = std::get_if<alias_fill_params::loc::at_reference_alias>(&fill)) {
         this->ui.fillTypeSiblingAlias->setChecked(true);
         set_combobox_to_alias(*this->ui.fillFromSiblingAlias, data->alias);
         this->ui.fillFromSiblingAliasParentKeyword->setFormStub(data->keyword.get_form_stub());
      } else if (auto* data = std::get_if<alias_fill_params::copy_external_alias>(&fill)) {
         this->ui.fillTypeExternalAlias->setChecked(true);
         this->ui.fillFromExtAliasQuest->setFormStub(data->quest.get_form_stub());
         this->_update_ext_alias_combobox();
         set_combobox_to_alias(*this->ui.fillFromExtAliasName, data->alias);
      } else if (const auto* data = std::get_if<alias_fill_params::loc::find>(&fill)) {
         this->ui.fillTypeMatching->setChecked(true);
         if (data->from_event.has_value()) {
            this->ui.findMatchingFlagEvent->setChecked(true);
            {
               auto& ev     = *data->from_event;
               auto* widget = this->ui.findMatchingEventData;
               if (ev.code == this->_data.quest.event) {
                  int i = widget->findData(ev.member);
                  if (i < 0)
                     i = 0;
                  widget->setCurrentIndex(i);
               } else {
                  widget->setCurrentIndex(0); // "NONE"
               }
            }
         } else {
            this->ui.findMatchingFlagEvent->setChecked(false);
         }
      }
   }
   this->ui.conditions->importFrom(this->_data.quest, this->_data.alias.conditions);
   this->ui.scriptListPane->setQuestWorkingCopyAndAliasVMAD(&this->_data.quest, this->_data.alias.script_data);
}
void FormSubdialogQuestLocAlias::save() {
   this->_data.alias.name = this->ui.name->text().toStdString();
   {
      auto& flags = this->_data.alias.flags;
      cobb::edit_bit(flags, loaded_alias_type::flag::reserves_target, this->ui.flagReserve->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::optional, this->ui.flagOptional->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::uses_stored_text, this->ui.flagDisplaysText->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::allow_reuse_in_quest, this->ui.flagAllowReuse->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::allow_reserved, this->ui.flagAllowReserved->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::allow_cleared, this->ui.flagAllowCleared->isChecked());
   }
   this->_data.alias.force_into_alias_id = this->ui.forceInto->currentData().toInt();

   auto& form = this->_data.quest;
   this->_data.alias.clear_fill_params(form);
   if (this->ui.fillTypeExternalAlias->isChecked()) {
      auto& dst = this->_data.alias.fill_params.emplace<alias_fill_params::copy_external_alias>();
      dst.quest.set(form, this->ui.fillFromExtAliasQuest->formStub());
      dst.alias = this->ui.fillFromExtAliasName->currentData().toInt();
   } else if (this->ui.fillTypeMatching->isChecked()) {
      auto& dst = this->_data.alias.fill_params.emplace<alias_fill_params::loc::find>();
      if (this->ui.findMatchingFlagEvent->isChecked()) {
         auto& ev = dst.from_event.emplace();
         ev.code   = form.event;
         ev.member = this->ui.findMatchingEventData->currentData().toInt();
      } else {
         dst.from_event = {};
      }
   } else if (this->ui.fillTypePredefined->isChecked()) {
      auto& dst = this->_data.alias.fill_params.emplace<alias_fill_params::loc::preassigned>();
      dst.location.set(form, this->ui.fillFromPredefined->formStub());
   } else if (this->ui.fillTypeSiblingAlias->isChecked()) {
      auto& dst = this->_data.alias.fill_params.emplace<alias_fill_params::loc::at_reference_alias>();
      dst.alias = this->ui.fillFromSiblingAlias->currentData().toInt();
      dst.keyword.set(form, this->ui.fillFromSiblingAliasParentKeyword->formStub());
   }
   this->ui.conditions->exportTo(this->_data.quest, this->_data.alias.conditions);
   this->ui.scriptListPane->commit();
}

void FormSubdialogQuestLocAlias::_update_ext_alias_combobox() {
   this->ui.fillFromExtAliasName->clear();

   auto* stub = this->ui.fillFromExtAliasQuest->formStub();
   if (!stub || stub->form_type != dovah::form_type::quest) {
      return;
   }
   auto loaded = stub->load().ptr_cast<loaded_form_type>();
   if (!loaded) {
      return;
   }
   make_location_alias_combobox(*loaded, *this->ui.fillFromExtAliasName);
}