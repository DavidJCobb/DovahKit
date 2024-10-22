#include "./FormSubdialogQuestRefAlias.h"
#include "dovah/data/story_manager.h"
#include "dovah/forms/components/papyrus/attached_script.h"
#include "dovah/forms/Quest.h"
#include "./alias_subdialog_helpers.h"

namespace {
   constexpr const auto no_alias = FormSubdialogQuestRefAlias::loaded_alias_type::none_id;
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

   static_assert(false, "TODO: Fill From: Specific Reference");
   static_assert(false, "TODO: Fill From: Unique Actor");
   static_assert(false, "TODO: Fill From: Location Alias");
   static_assert(false, "TODO: Fill From: Location Alias: LocRefType");
   this->ui.fillFromExtAliasQuest->setAllowedFormType(dovah::form_type::quest);
   static_assert(false, "TODO: Fill From: Create Reference");
   make_event_data_comboboxes(quest, *this->ui.findMatchingEventName, *this->ui.findMatchingEventData);
   make_reference_alias_combobox(quest, *this->ui.findMatchingNearAliasID);
   static_assert(false, "TODO: Fill From: Matching Reference: Near Alias: Near Type"); // always "Linked Ref Child"?

   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, [this]() {
      this->save();
      this->accept();
   });
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

   this->load();
}

void FormSubdialogQuestRefAlias::load() {
   this->ui.name->setText(QString::fromStdString(this->_data.alias.name));
   static_assert(false, "TODO: Display Name");
   static_assert(false, "TODO: Additional Voicetypes for Export");
   {
      const auto flags = this->_data.alias.flags;
      this->ui.flagReserve->setChecked(flags & loaded_alias_type::flag::reserves_target);
      this->ui.flagOptional->setChecked(flags & loaded_alias_type::flag::optional);
      this->ui.flagDisplaysText->setChecked(flags & loaded_alias_type::flag::uses_stored_text);
      this->ui.flagAllowReuse->setChecked(flags & loaded_alias_type::flag::allow_reuse_in_quest);
      this->ui.flagAllowReserved->setChecked(flags & loaded_alias_type::flag::allow_reserved);
      this->ui.flagAllowCleared->setChecked(flags & loaded_alias_type::flag::allow_cleared);
      static_assert(false, "TODO: Flags");
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
      switch (this->_data.alias.fill_type) {
         using fill_type = decltype(this->_data.alias.fill_type);
         static_assert(false, "TODO: Fill From: Specific Reference");
         static_assert(false, "TODO: Fill From: Unique Actor");
         static_assert(false, "TODO: Fill From: Location Alias");
         static_assert(false, "TODO: Fill From: Location Alias: LocRefType");
         static_assert(false, "TODO: Fill From: Create Reference");
         static_assert(false, "TODO: Fill From: Matching Reference: Near Alias: Near Type"); // always "Linked Ref Child"?

         case fill_type::other_alias_in_same_quest:
            this->ui.fillTypeSiblingAlias->setChecked(true);
            set_combobox_to_alias(
               *this->ui.fillFromSiblingAlias,
               this->_data.alias.fill_from_alias.alias
            );
            this->ui.fillFromSiblingAliasParentKeyword->setFormStub(this->_data.alias.fill_from_location_keyword.get_form_stub());
            break;
         case fill_type::from_event:
            this->ui.fillTypeMatching->setChecked(true);
            this->ui.findMatchingModeEvent->setChecked(true);
            {
               auto* widget = this->ui.findMatchingEventData;
               if (this->_data.alias.fill_from_event.code == this->_data.quest.event) {
                  int i = widget->findData(this->_data.alias.fill_from_event.member);
                  if (i < 0)
                     i = 0;
                  widget->setCurrentIndex(i);
               } else {
                  widget->setCurrentIndex(0); // "NONE"
               }
            }
            break;
         case fill_type::other_alias_in_other_quest:
            this->ui.fillTypeExternalAlias->setChecked(true);
            this->ui.fillFromExtAliasQuest->setFormStub(this->_data.alias.fill_from_alias.quest.get_form_stub());
            this->_update_ext_alias_combobox();
            set_combobox_to_alias(*this->ui.fillFromExtAliasName, this->_data.alias.fill_from_alias.alias);
            break;
         default:
         case fill_type::preset_placed_reference:
            this->ui.fillTypePredefined->setChecked(true);
            this->ui.fillFromPredefined->setFormStub(this->_data.alias.fill_from_reference.get_form_stub());
            break;
         case fill_type::find_matching_reference:
            this->ui.fillTypeMatching->setChecked(true);
            this->ui.findMatchingModeEvent->setChecked(false);
            break;
      }
   }
   this->ui.findMatchingConditions->importFrom(this->_data.quest, this->_data.alias.conditions);
   this->ui.scriptListPane->setQuestWorkingCopyAndAliasVMAD(&this->_data.quest, this->_data.alias.script_data);
}
void FormSubdialogQuestRefAlias::save() {
   auto& form = this->_data.quest;

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
   this->ui.factions->commitStubs(this->_data.alias.factions);
   this->ui.keywords->commitStubs(this->_data.alias.keywords.forms);
   this->ui.spells->commitStubs(this->_data.alias.spells);
   this->ui.packages->commitStubs(this->_data.alias.packages);
   this->_data.alias.package_override_lists.combat.set(form, this->ui.packageOverrideListCombat->formStub());
   this->_data.alias.package_override_lists.guard_warn.set(form, this->ui.packageOverrideListGuardWarn->formStub());
   this->_data.alias.package_override_lists.spectator.set(form, this->ui.packageOverrideListSpectator->formStub());
   this->_data.alias.package_override_lists.observe_corpse.set(form, this->ui.packageOverrideListObserveCorpse->formStub());
   this->ui.inventory->commitTo(this->_data.alias.inventory);

   using fill_type = decltype(this->_data.alias.fill_type);

   auto& form = this->_data.quest;
   if (this->ui.fillTypeExternalAlias->isChecked()) {
      this->_data.alias.fill_type = fill_type::other_alias_in_other_quest;
      this->_data.alias.fill_from_alias.quest.set(form, this->ui.fillFromExtAliasQuest->formStub());
      this->_data.alias.fill_from_alias.alias = this->ui.fillFromExtAliasName->currentData().toInt();
   } else if (this->ui.fillTypeMatching->isChecked()) {
      if (this->ui.findMatchingFlagEvent->isChecked()) {
         this->_data.alias.fill_type = fill_type::from_event;
         this->_data.alias.fill_from_event.code   = form.event;
         this->_data.alias.fill_from_event.member = this->ui.findMatchingEventData->currentData().toInt();
      } else {
         this->_data.alias.fill_type = fill_type::find_matching_reference;
         this->_data.alias.fill_from_event = {};
      }
   } else if (this->ui.fillTypePredefined->isChecked()) {
      this->_data.alias.fill_type = fill_type::preset_location;
      this->_data.alias.fill_from_location.set(form, this->ui.fillFromPredefined->formStub());
   } else if (this->ui.fillTypeSiblingAlias->isChecked()) {
      this->_data.alias.fill_type = fill_type::other_alias_in_same_quest;
      this->_data.alias.fill_from_alias.alias = this->ui.fillFromSiblingAlias->currentData().toInt();
      this->_data.alias.fill_from_location_keyword.set(form, this->ui.fillFromSiblingAliasParentKeyword->formStub());
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