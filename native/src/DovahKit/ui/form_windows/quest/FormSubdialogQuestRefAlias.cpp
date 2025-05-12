#include "./FormSubdialogQuestRefAlias.h"
#include "dovah/data/story_manager.h"
#include "dovah/forms/components/papyrus/attached_script.h"
#include "dovah/forms/Quest.h"
#include "./alias_subdialog_helpers.h"

namespace {
   constexpr const auto no_alias = FormSubdialogQuestRefAlias::loaded_alias_type::none_id;
   namespace alias_fill_params {
      using namespace dovah::loaded_forms::structs::alias_fill_params;
   }
   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
   }
}

FormSubdialogQuestRefAlias::FormSubdialogQuestRefAlias(loaded_form_type& quest, loaded_alias_type& alias, QWidget* parent)
   :
   QDialog(parent),
   _data{ quest, alias }
{
   this->ui.setupUi(this);

   this->ui.displayName->setAllowedFormType(dovah::form_type::message);
   this->ui.additionalVoicetypes->setAllowedFormType(dovah::form_type::actor_base);
   make_location_alias_combobox(quest, *this->ui.forceInto);

   this->ui.factions->setAllowedFormTypes({ dovah::form_type::faction });
   this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });
   this->ui.spells->setAllowedFormTypes({ dovah::form_type::spell, dovah::form_type::shout });
   this->ui.packages->setAllowedFormTypes({ dovah::form_type::package });
   this->ui.packageOverrideListCombat->setAllowedFormType(dovah::form_type::formlist);
   this->ui.packageOverrideListGuardWarn->setAllowedFormType(dovah::form_type::formlist);
   this->ui.packageOverrideListSpectator->setAllowedFormType(dovah::form_type::formlist);
   this->ui.packageOverrideListObserveCorpse->setAllowedFormType(dovah::form_type::formlist);

   this->ui.fillFromUniqueActorBase->setAllowedFormType(dovah::form_type::actor_base);
   //
   {
      auto* widget = this->ui.createLevel;
      widget->clear();
      widget->addItem(tr("Easy"), 0);
      widget->addItem(tr("Medium"), 1);
      widget->addItem(tr("Hard"), 2);
      widget->addItem(tr("Very Hard"), 3);
      widget->addItem(tr("None"), 4);
   }
   {
      auto* widget = this->ui.createVerb;
      widget->clear();
      widget->addItem(tr("at"), 0x0000);
      widget->addItem(tr("in"), 0x8000);
   }
   make_reference_alias_combobox(quest, *this->ui.createAtSiblingReferenceAlias);
   //
   make_location_alias_combobox(quest, *this->ui.fillFromLocationAlias);
   this->ui.fillFromLocationRefType->setAllowedFormType(dovah::form_type::location_ref_type);
   //
   this->ui.fillFromExtAliasQuest->setAllowedFormType(dovah::form_type::quest);
   //
   make_event_data_comboboxes(quest, *this->ui.findMatchingEventName, *this->ui.findMatchingEventData);
   make_reference_alias_combobox(quest, *this->ui.findMatchingNearAliasID);
   {
      auto* widget = this->ui.findMatchingNearType;
      widget->clear();
      widget->addItem(tr("Linked Ref Child"), 0);
   }

   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, [this]() {
      this->save();
      this->accept();
   });
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

   this->load();
}

void FormSubdialogQuestRefAlias::load() {
   this->ui.name->setText(QString::fromStdString(this->_data.alias.name));
   this->ui.displayName->setFormStub(this->_data.alias.display_name.get_form_stub());
   this->ui.additionalVoicetypes->setFormStub(this->_data.alias.additional_voicetype.get_form_stub());
   {
      const auto flags = this->_data.alias.flags;
      this->ui.flagReserve->setChecked(flags & loaded_alias_type::flag::reserves_target);
      this->ui.flagOptional->setChecked(flags & loaded_alias_type::flag::optional);
      this->ui.flagEssential->setChecked(flags & loaded_alias_type::flag::make_essential);
      this->ui.flagProtected->setChecked(flags & loaded_alias_type::flag::make_protected);
      this->ui.flagQuestObject->setChecked(flags & loaded_alias_type::flag::quest_object);
      //
      this->ui.flagAllowReuse->setChecked(flags & loaded_alias_type::flag::allow_reuse_in_quest);
      this->ui.flagAllowDead->setChecked(flags & loaded_alias_type::flag::allow_dead);
      this->ui.flagAllowDisabled->setChecked(flags & loaded_alias_type::flag::allow_disabled);
      this->ui.flagAllowReserved->setChecked(flags & loaded_alias_type::flag::allow_reserved);
      this->ui.flagAllowDestroyed->setChecked(flags & loaded_alias_type::flag::allow_destroyed);
      //
      this->ui.flagUseStoredText->setChecked(flags & loaded_alias_type::flag::uses_stored_text);
      this->ui.flagStoreText->setChecked(flags & loaded_alias_type::flag::stores_text);
      this->ui.flagInitiallyDisabled->setChecked(flags & loaded_alias_type::flag::initially_disabled);
      this->ui.flagClearNameWhenRemoved->setChecked(flags & loaded_alias_type::flag::clear_name_when_removed);
   }
   this->ui.factions->pullStubs(this->_data.alias.factions);
   this->ui.keywords->pullStubs(this->_data.alias.keywords.forms);
   this->ui.spells->pullStubs(this->_data.alias.spells);
   this->ui.packages->pullStubs(this->_data.alias.packages);
   this->ui.packageOverrideListCombat->setFormStub(this->_data.alias.package_override_lists.combat.get_form_stub());
   this->ui.packageOverrideListGuardWarn->setFormStub(this->_data.alias.package_override_lists.guard_warn.get_form_stub());
   this->ui.packageOverrideListSpectator->setFormStub(this->_data.alias.package_override_lists.spectator.get_form_stub());
   this->ui.packageOverrideListObserveCorpse->setFormStub(this->_data.alias.package_override_lists.observe_corpse.get_form_stub());
   this->ui.inventory->initializeFrom(this->_data.alias.inventory);

   {
      auto* widget   = this->ui.forceInto;
      auto  alias_id = this->_data.alias.force_into_alias_id;
      set_combobox_to_alias(*widget, alias_id);
   }
   {  // Fill type
      using alias_flag = std::decay_t<decltype(this->_data.alias)>::flag::type;

      auto& fill = this->_data.alias.fill_params;
      if (const auto* data = std::get_if<alias_fill_params::ref::preassigned>(&fill)) {
         this->ui.fillTypePredefined->setChecked(true);
         this->ui.fillFromPredefined->setRef(data->ref.get_form_stub());
      } else if (const auto* data = std::get_if<alias_fill_params::ref::unique_actor>(&fill)) {
         this->ui.fillTypeUniqueActor->setChecked(true);
         this->ui.fillFromUniqueActorBase->setFormStub(data->actor_base.get_form_stub());
      } else if (const auto* data = std::get_if<alias_fill_params::ref::at_location_alias>(&fill)) {
         this->ui.fillTypeLocRefType->setChecked(true);
         set_combobox_to_alias(*this->ui.fillFromLocationAlias, data->alias);
         this->ui.fillFromLocationRefType->setFormStub(data->loc_ref_type.get_form_stub());
      } else if (const auto* data = std::get_if<alias_fill_params::copy_external_alias>(&fill)) {
         this->ui.fillTypeExternalAlias->setChecked(true);
         this->ui.fillFromExtAliasQuest->setFormStub(data->quest.get_form_stub());
         this->_update_ext_alias_combobox();
         set_combobox_to_alias(*this->ui.fillFromExtAliasID, data->alias);
      } else if (const auto* data = std::get_if<alias_fill_params::ref::create>(&fill)) {
         this->ui.createBaseForm->setFormStub(data->base_form.get_form_stub());
         {
            auto* widget = this->ui.createLevel;
            auto  i = widget->findData(data->difficulty);
            if (i < 0)
               i = widget->findData(4); // "None"
            widget->setCurrentIndex(i);
         }
         {
            auto* widget = this->ui.createVerb;
            auto  i = widget->findData(data->at_reference.place_in_inventory ? 1 : 0);
            if (i < 0)
               i = 0;
            widget->setCurrentIndex(i);
         }
         set_combobox_to_alias(*this->ui.createAtSiblingReferenceAlias, data->at_reference.alias);
      } else if (const auto* data = std::get_if<alias_fill_params::ref::find_in_loaded_area>(&fill)) {
         this->ui.fillTypeMatching->setChecked(true);

         this->ui.findMatchingModeLoadedArea->setChecked(true);
         this->ui.findMatchingModeEvent->setChecked(false);
         this->ui.findMatchingModeNearAlias->setChecked(false);

         this->ui.findMatchingFlagClosest->setChecked(data->closest);
      } else if (const auto* data = std::get_if<alias_fill_params::ref::find_from_event>(&fill)) {
         this->ui.fillTypeMatching->setChecked(true);

         this->ui.findMatchingModeLoadedArea->setChecked(false);
         this->ui.findMatchingModeNearAlias->setChecked(false);
         this->ui.findMatchingModeEvent->setChecked(true);

         {
            auto* widget = this->ui.findMatchingEventData;
            if (data->code == this->_data.quest.event) {
               int i = widget->findData(data->member);
               if (i < 0)
                  i = 0;
               widget->setCurrentIndex(i);
            } else {
               widget->setCurrentIndex(0); // "NONE"
            }
         }
      } else if (const auto* data = std::get_if<alias_fill_params::ref::find_near_alias>(&fill)) {
         this->ui.fillTypeMatching->setChecked(true);

         this->ui.findMatchingModeLoadedArea->setChecked(false);
         this->ui.findMatchingModeEvent->setChecked(false);
         this->ui.findMatchingModeNearAlias->setChecked(true);

         set_combobox_to_alias(*this->ui.findMatchingNearAliasID, data->alias);
         {
            auto* widget = this->ui.findMatchingNearType;
            int   i = widget->findData((int)data->near_type);
            if (i >= 0)
               widget->setCurrentIndex(i);
         }
      } else {
         this->ui.fillTypePredefined->setChecked(true);
      }
   }
   this->ui.findMatchingConditions->importFrom(this->_data.quest, this->_data.alias.conditions);
   this->ui.scriptListPane->setQuestWorkingCopyAndAliasVMAD(&this->_data.quest, this->_data.alias.script_data);
}
void FormSubdialogQuestRefAlias::save() {
   auto& form = this->_data.quest;

   this->_data.alias.name = this->ui.name->text().toStdString();
   this->_data.alias.display_name.set(form, this->ui.displayName->formStub());
   this->_data.alias.additional_voicetype.set(form, this->ui.additionalVoicetypes->formStub());
   {
      auto& flags = this->_data.alias.flags;
      cobb::edit_bit(flags, loaded_alias_type::flag::reserves_target, this->ui.flagReserve->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::optional, this->ui.flagOptional->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::make_essential, this->ui.flagEssential->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::make_protected, this->ui.flagProtected->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::quest_object, this->ui.flagQuestObject->isChecked());
      //
      cobb::edit_bit(flags, loaded_alias_type::flag::allow_reuse_in_quest, this->ui.flagAllowReuse->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::allow_dead, this->ui.flagAllowDead->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::allow_disabled, this->ui.flagAllowDisabled->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::allow_reserved, this->ui.flagAllowReserved->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::allow_destroyed, this->ui.flagAllowDestroyed->isChecked());
      //
      cobb::edit_bit(flags, loaded_alias_type::flag::uses_stored_text, this->ui.flagUseStoredText->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::stores_text, this->ui.flagStoreText->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::initially_disabled, this->ui.flagInitiallyDisabled->isChecked());
      cobb::edit_bit(flags, loaded_alias_type::flag::clear_name_when_removed, this->ui.flagClearNameWhenRemoved->isChecked());
   }
   this->_data.alias.force_into_alias_id = this->ui.forceInto->currentData().toInt();
   this->ui.factions->commitStubs(this->_data.alias.factions, this->_data.quest);
   this->ui.keywords->commitStubs(this->_data.alias.keywords.forms, this->_data.quest);
   this->ui.spells->commitStubs(this->_data.alias.spells, this->_data.quest);
   this->ui.packages->commitStubs(this->_data.alias.packages, this->_data.quest);
   this->_data.alias.package_override_lists.combat.set(form, this->ui.packageOverrideListCombat->formStub());
   this->_data.alias.package_override_lists.guard_warn.set(form, this->ui.packageOverrideListGuardWarn->formStub());
   this->_data.alias.package_override_lists.spectator.set(form, this->ui.packageOverrideListSpectator->formStub());
   this->_data.alias.package_override_lists.observe_corpse.set(form, this->ui.packageOverrideListObserveCorpse->formStub());
   this->ui.inventory->commitTo(this->_data.alias.inventory, form);

   using alias_flag = std::decay_t<decltype(this->_data.alias)>::flag::type;

   auto& form = this->_data.quest;
   this->_data.alias.clear_fill_params(form);
   if (this->ui.fillTypePredefined->isChecked()) {
      auto& dst = this->_data.alias.fill_params.emplace<alias_fill_params::ref::preassigned>();
      dst.ref.set(form, this->ui.fillFromPredefined->ref());
   } else if (this->ui.fillTypeUniqueActor->isChecked()) {
      auto& dst = this->_data.alias.fill_params.emplace<alias_fill_params::ref::unique_actor>();
      dst.actor_base.set(form, this->ui.fillFromUniqueActorBase->formStub());
   } else if (this->ui.fillTypeLocRefType->isChecked()) {
      auto& dst = this->_data.alias.fill_params.emplace<alias_fill_params::ref::at_location_alias>();
      dst.alias = this->ui.fillFromLocationAlias->currentData().toInt();
      dst.loc_ref_type.set(form, this->ui.fillFromLocationRefType->formStub());
   } else if (this->ui.fillTypeExternalAlias->isChecked()) {
      auto& dst = this->_data.alias.fill_params.emplace<alias_fill_params::copy_external_alias>();
      dst.quest.set(form, this->ui.fillFromExtAliasQuest->formStub());
      dst.alias = this->ui.fillFromExtAliasID->currentData().toInt();
   } else if (this->ui.fillTypeCreate->isChecked()) {
      auto& dst = this->_data.alias.fill_params.emplace<alias_fill_params::ref::create>();
      dst.base_form.set(form, this->ui.createBaseForm->formStub());
      dst.difficulty = this->ui.createLevel->currentData().toInt();;
      dst.at_reference.alias = this->ui.createAtSiblingReferenceAlias->currentData().toInt();
      dst.at_reference.place_in_inventory = this->ui.createVerb->currentData().toInt() != 0;
   } else if (this->ui.fillTypeMatching->isChecked()) {
      if (this->ui.findMatchingModeEvent->isChecked()) {
         auto& dst = this->_data.alias.fill_params.emplace<alias_fill_params::ref::find_from_event>();
         dst.code   = form.event;
         dst.member = this->ui.findMatchingEventData->currentData().toInt();
      } else if (this->ui.findMatchingModeLoadedArea->isChecked()) {
         auto& dst = this->_data.alias.fill_params.emplace<alias_fill_params::ref::find_in_loaded_area>();
         dst.closest = this->ui.findMatchingFlagClosest->isChecked();
      } else {
         auto& dst = this->_data.alias.fill_params.emplace<alias_fill_params::ref::find_near_alias>();
         dst.alias     = this->ui.findMatchingNearAliasID->currentData().toInt();
         dst.near_type = this->ui.findMatchingNearType->currentData().toInt();
      }
   }
   this->ui.findMatchingConditions->exportTo(this->_data.quest, this->_data.alias.conditions);
   this->ui.scriptListPane->commit();
}

void FormSubdialogQuestRefAlias::_update_ext_alias_combobox() {
   this->ui.fillFromExtAliasID->clear();

   auto* stub = this->ui.fillFromExtAliasQuest->formStub();
   if (!stub || stub->form_type != dovah::form_type::quest) {
      return;
   }
   auto loaded = stub->load().ptr_cast<loaded_form_type>();
   if (!loaded) {
      return;
   }
   make_reference_alias_combobox(*loaded, *this->ui.fillFromExtAliasID);
}