#include "./encounter_zone.h"
#include "ui/utils/bind.h"

FormDialogEncounterZone::FormDialogEncounterZone(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   for (auto* widget : std::array{
      this->ui.levelMin,
      this->ui.levelMax,
   }) {
      widget->setRange(0, 100);
   }
   
   this->ui.location->setAllowedFormType(dovah::form_type::location);

   QObject::connect(this->ui.ownerTypeNPC, &QRadioButton::toggled, this, [this](bool checked) {
      this->ui.ownerNPC->setEnabled(checked);
      this->ui.ownerFaction->setEnabled(!checked);
      this->ui.ownerFactionRank->setEnabled(!checked);
   });
   {
      bool checked = this->ui.ownerTypeNPC->isChecked();
      this->ui.ownerNPC->setEnabled(checked);
      this->ui.ownerFaction->setEnabled(!checked);
      this->ui.ownerFactionRank->setEnabled(!checked);
   }
   this->ui.ownerNPC->setAllowedFormType(dovah::form_type::actor_base);
   this->ui.ownerFaction->setAllowedFormType(dovah::form_type::faction);
   this->ui.ownerNPC->setAllowNone(true);
   this->ui.ownerFaction->setAllowNone(true);
   QObject::connect(this->ui.ownerFaction, &DKFormPicker::formChanged, this, [this]() {
      auto& editor = DovahKitCore::get();
      this->working_ownership.form = this->ui.ownerFaction->formStub();
      this->_update_ownership_widgets();
   });
   QObject::connect(this->ui.ownerNPC, &DKFormPicker::formChanged, this, [this]() {
      auto& editor = DovahKitCore::get();
      this->working_ownership.form = this->ui.ownerNPC->formStub();
      this->_update_ownership_widgets();
   });

   this->load(); // this creates the working copy.
}
void FormDialogEncounterZone::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.levelMin, working.data.min_level);
   ui::bind(this->ui.levelMax, working.data.max_level);
   ui::bind(this->ui.location, working.location, working);
   {  // Flags
      ui::bind(this->ui.flagMatchBelowMin, working.data.flags, loaded_form_type::flag::match_pc_below_min_level);
      ui::bind(this->ui.flagNeverResets, working.data.flags, loaded_form_type::flag::never_resets);
      ui::bind(this->ui.flagDisableCombatBoundary, working.data.flags, loaded_form_type::flag::disable_combat_boundary);
   }
   {  // Ownership
      this->ui.ownerFactionRank->clear();
      this->working_ownership.form = working.owner.get_form_stub();
      this->working_ownership.rank = working.data.rank;
      bool is_npc = true;
      if (auto* stub = this->working_ownership.form) {
         is_npc = stub->form_type != dovah::form_type::faction;
      }
      this->ui.ownerTypeNPC->setChecked(is_npc);
      this->ui.ownerTypeFaction->setChecked(!is_npc);
      this->_update_ownership_widgets();
   }
}
void FormDialogEncounterZone::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   if (auto* stub = this->working_ownership.form) {
      this->write_form_ref(working.owner, stub);
      if (stub->form_type == dovah::form_type::faction) {
         working.data.rank = this->working_ownership.rank;
      }
   } else {
      this->write_form_ref(working.owner, nullptr);
   }
}
void FormDialogEncounterZone::_update_ownership_widgets() {
   const auto blocker0 = QSignalBlocker(this->ui.ownerFaction);
   const auto blocker1 = QSignalBlocker(this->ui.ownerFactionRank);
   const auto blocker2 = QSignalBlocker(this->ui.ownerNPC);
   //
   dovah::form_stub* faction_stub = nullptr;
   //
   this->ui.ownerFactionRank->clear();
   //
   this->ui.ownerFaction->setEnabled(true);
   this->ui.ownerNPC->setEnabled(true);
   this->ui.ownerFactionRank->setEnabled(true);
   this->ui.ownerFaction->setFormStub(this->working_ownership.form); // the control will filter for us
   this->ui.ownerNPC->setFormStub(this->working_ownership.form); // the control will filter for us
   if (auto* stub = this->working_ownership.form) {
      if (stub->form_type == dovah::form_type::faction)
         faction_stub = stub;
   }
   //
   {
      dovah::form_stub* prior_stub = nullptr;
      if (this->working_ownership.loaded_faction)
         prior_stub = &this->working_ownership.loaded_faction->stub;
      //
      if (prior_stub != faction_stub) {
         if (faction_stub)
            this->working_ownership.loaded_faction = faction_stub->load().ptr_cast<dovah::loaded_forms::Faction>();
         else
            this->working_ownership.loaded_faction = nullptr;
         this->_update_rank_picker();
      }
   }
}
void FormDialogEncounterZone::_update_rank_picker() {
   auto* widget = this->ui.ownerFactionRank;
   const auto blocker = QSignalBlocker(widget);
   if (this->working_ownership.loaded_faction) {
      widget->setEnabled(true);
      widget->clear();
      //
      auto& list = this->working_ownership.loaded_faction->ranks;
      for (auto& rank : list) {
         QString fem  = rank.title_fem.c_str();
         QString masc = rank.title_masc.c_str();
         //
         QString text = fem;
         if (masc != fem) {
            text = tr((const char*)u8"%1 (\x2640) / %2 (\x2642)", "ownership required rank").arg(fem).arg(masc);
         }
         widget->addItem(text, rank.id);
      }
   } else {
      widget->setEnabled(false);
      widget->clear();
   }
}