#include "quest.h"
#include "_base_cpp.h"
#include "../../dovah/core.h"
#include "../../helpers/qt/basic_bindings.h"

FormDialogQuest::FormDialogQuest(dovah::form_stub* stub, QWidget* parent) : FormDialogWorkingCopyBase(dovah::form_type::quest, stub, parent) {
   {
      auto* widget = this->ui.textDisplayGlobals;
      auto* model  = widget->fullModel();
      this->ui.textDisplayGlobals->setAcceptDrops(true);
      model->setAllowGaps(false);
      model->setAllowedFormTypes({ dovah::form_type::global });
   }
   {
      using _e = form_t::quest_type::type;
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
   auto& editor  = DovahKitCore::get();
   auto& working = *this->get_working_copy<form_t>();
   //
   #pragma region Basic Data
      this->ui.editorID->setText(QString::fromStdString(this->stub->get_editor_id()));
      cobb::qt::bind(this->ui.editorCategory, working.editor_category);
      this->ui.name->setText(editor.convert_localized_string(working.name));
      cobb::qt::bind(this->ui.questType, working.quest_type);
      cobb::qt::bind(this->ui.flagAllowRepeatedStages, working.flags, form_t::quest_flag::allow_repeated_stages);
      cobb::qt::bind(this->ui.flagExcludeFromDialogueExport, working.flags, form_t::quest_flag::exclude_from_dialogue_export);
      cobb::qt::bind(this->ui.flagRunOnce, working.flags, form_t::quest_flag::run_once);
      cobb::qt::bind(this->ui.flagStartGameEnabled, working.flags, form_t::quest_flag::start_game_enabled);
      cobb::qt::bind(this->ui.flagWarnOnAliasFillFailure, working.flags, form_t::quest_flag::warn_on_alias_fill_failure);
      //
      // TODO: Event
      //
      this->ui.textDisplayGlobals->import(working.text_display_globals);
      //
      cobb::qt::bind(this->ui.priority, working.priority);
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
   //
   // FormDialogWorkingCopyBase will handle the task of saving the working copy. 
   // We just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->get_working_copy<form_t>();
   //
   this->stub->editorID = this->ui.editorID->text().toStdString();
   editor.assign_localized_string(working.name, this->ui.name->text());
   this->ui.textDisplayGlobals->commit(working.text_display_globals, working);

   // TODO: EVERYTHING THAT DOESN'T MODIFY THE WORKING COPY IN REAL-TIME
}