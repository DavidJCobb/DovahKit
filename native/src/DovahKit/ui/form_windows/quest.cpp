#include "quest.h"
#include "_base_cpp.h"
#include "../../dovah/core.h"

FormDialogQuest::FormDialogQuest(dovah::form_stub* stub, QWidget* parent) : FormDialogBaseTemplate(stub, parent) {
   form_dialog_helpers::initialize<FormDialogQuest, dovah::loaded_forms::Quest>(*this, stub);
   //
   {
      auto* widget = this->ui.textDisplayGlobals;
      auto* model  = widget->fullModel();
      this->ui.textDisplayGlobals->setAcceptDrops(true);
      model->setAllowGaps(false);
      model->setAllowedFormTypes({ dovah::form_type::global });
   }
   {
      using _e = dovah::loaded_forms::Quest::quest_type::type;
      this->ui.questType->addItem(tr("None",                  "Quest Type"), _e::none);
      this->ui.questType->addItem(tr("Main",                  "Quest Type"), _e::main);
      this->ui.questType->addItem(tr("College of Winterhold", "Quest Type"), _e::mages_guild);
      this->ui.questType->addItem(tr("Thieves Guild",         "Quest Type"), _e::thieves_guild);
      this->ui.questType->addItem(tr("dark_brotherhood",      "Quest Type"), _e::dark_brotherhood);
      this->ui.questType->addItem(tr("Companions",            "Quest Type"), _e::companions);
      this->ui.questType->addItem(tr("Miscellaneous",         "Quest Type"), _e::miscellaneous);
      this->ui.questType->addItem(tr("Daedric",               "Quest Type"), _e::daedric);
      this->ui.questType->addItem(tr("Sidequest",             "Quest Type"), _e::sidequest);
      this->ui.questType->addItem(tr("civil_war",             "Quest Type"), _e::civil_war);
      this->ui.questType->addItem(tr("DLC: Dawnguard",        "Quest Type"), _e::dlc_dawnguard);
      this->ui.questType->addItem(tr("DLC: Dragonborn",       "Quest Type"), _e::dlc_dragonborn);
   }
   this->ui.priority->setRange(0, 255);
   //
   this->load();
   //
   auto& editor = DovahKitCore::get();
}
void FormDialogQuest::_load_impl() {
   auto& editor = DovahKitCore::get();
   //
   #pragma region Basic Data
      this->ui.editorID->setText(QString::fromStdString(this->stub->get_editor_id()));
      this->ui.editorCategory->setText(QString::fromStdString(this->form->editor_category));
      editor.assign_localized_string(this->form->name, this->ui.name->text());
      this->ui.questType->setCurrentIndex(this->ui.questType->findData(this->form->quest_type));
      //
      // TODO: Event
      //
      this->ui.flagAllowRepeatedStages->setChecked(this->form->flags & dovah::loaded_forms::Quest::quest_flag::allow_repeated_stages);
      this->ui.flagExcludeFromDialogueExport->setChecked(this->form->flags & dovah::loaded_forms::Quest::quest_flag::exclude_from_dialogue_export);
      this->ui.flagRunOnce->setChecked(this->form->flags & dovah::loaded_forms::Quest::quest_flag::run_once);
      this->ui.flagStartGameEnabled->setChecked(this->form->flags & dovah::loaded_forms::Quest::quest_flag::start_game_enabled);
      this->ui.flagWarnOnAliasFillFailure->setChecked(this->form->flags & dovah::loaded_forms::Quest::quest_flag::warn_on_alias_fill_failure);
      //
      {
         auto* widget = this->ui.textDisplayGlobals;
         auto& list   = this->form->text_display_globals;
         widget->clear();
         widget->reserve(list.size());
         for (auto& ref : list)
            widget->addStub(ref.get_form_stub());
      }
      //
      this->ui.priority->setValue(this->form->priority);
      //
      // TODO: Dialogue conditions
      //
   #pragma endregion
   #pragma region Stages

   #pragma endregion
   #pragma region Objectives
   #pragma endregion
   #pragma region Aliases
   #pragma endregion
   #pragma region Dialogue
   #pragma endregion
   #pragma region Scenes
   #pragma endregion
   #pragma region Scripts
   #pragma endregion
}
void FormDialogQuest::_save_impl() {
   auto& editor = DovahKitCore::get();
   //
   this->stub->editorID = this->ui.editorID->text().toStdString();
   //
   auto&  list  = this->form->contents;
   auto   stubs = this->ui.forms->stubs();
   size_t i     = 0;
   size_t size  = stubs.size();
   if (list.size() < size)
      list.resize(size);
   for (; i < size; ++i)
      list[i].set(*this->stub, stubs[i]);
   //
   // Delete excess elements, if any were removed:
   //
   auto s = list.size();
   if (s != size) {
      for (; i < s; ++i)
         list[i].set(*this->stub, nullptr);
      list.resize(size);
   }
}