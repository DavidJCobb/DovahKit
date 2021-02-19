#include "quest.h"
#include "_base_cpp.h"
#include "../../dovah/core.h"
#include "../../helpers/qt/basic_bindings.h"

FormDialogQuest::FormDialogQuest(dovah::form_stub* stub, QWidget* parent) : FormDialogWorkingCopyBase(dovah::form_type::quest, stub, parent) {
   form_dialog_helpers::initialize(*this);
   //
   this->ui.tabWidget->setCurrentIndex(0);
   this->ui.dialogueTabbox->setCurrentIndex(0);
   //
   {
      auto* widget = this->ui.textDisplayGlobals;
      auto* model  = widget->fullModel();
      widget->setAcceptDrops(true);
      model->setAllowGaps(false);
      model->setAllowedFormTypes({ dovah::form_type::global });
      model->setShowIndices(false);
   }
   {
      using _e = form_t::quest_type::type;
      this->ui.questType->addItem(tr("None",                  "Quest Type"), _e::none);
      this->ui.questType->addItem(tr("Main",                  "Quest Type"), _e::main);
      this->ui.questType->addItem(tr("College of Winterhold", "Quest Type"), _e::mages_guild);
      this->ui.questType->addItem(tr("Thieves Guild",         "Quest Type"), _e::thieves_guild);
      this->ui.questType->addItem(tr("Dark Brotherhood",      "Quest Type"), _e::dark_brotherhood);
      this->ui.questType->addItem(tr("Companions",            "Quest Type"), _e::companions);
      this->ui.questType->addItem(tr("Miscellaneous",         "Quest Type"), _e::miscellaneous);
      this->ui.questType->addItem(tr("Daedric",               "Quest Type"), _e::daedric);
      this->ui.questType->addItem(tr("Sidequest",             "Quest Type"), _e::sidequest);
      this->ui.questType->addItem(tr("Civil War",             "Quest Type"), _e::civil_war);
      this->ui.questType->addItem(tr("DLC: Dawnguard",        "Quest Type"), _e::dlc_dawnguard);
      this->ui.questType->addItem(tr("DLC: Dragonborn",       "Quest Type"), _e::dlc_dragonborn);
   }
   {
      using _e = dovah::story_event_code::type;
      auto* widget = this->ui.eventType;
      widget->addItem(tr("None", "SM event name"), _e::none);
      widget->addItem(tr("Crime Gold", "SM event name"), _e::crime_gold);
      widget->addItem(tr("Actor Dialogue", "SM event name"), _e::actor_dialogue);
      widget->addItem(tr("Player Activate Actor", "SM event name"), _e::player_activate_actor);
      widget->addItem(tr("Actor Hello", "SM event name"), _e::actor_hello);
      widget->addItem(tr("Player Add Item", "SM event name"), _e::player_add_item);
      widget->addItem(tr("Arrest", "SM event name"), _e::arrest);
      widget->addItem(tr("Assault", "SM event name"), _e::assault);
      widget->addItem(tr("Bribe", "SM event name"), _e::bribe);
      widget->addItem(tr("Cast Magic", "SM event name"), _e::cast_magic);
      widget->addItem(tr("Change Relationship Rank", "SM event name"), _e::change_relationship_rank);
      widget->addItem(tr("Change Location", "SM event name"), _e::change_location);
      widget->addItem(tr("Craft Item", "SM event name"), _e::craft_item);
      widget->addItem(tr("Player Cured", "SM event name"), _e::player_cured);
      widget->addItem(tr("Dead Body", "SM event name"), _e::dead_body);
      widget->addItem(tr("Escaped Jail", "SM event name"), _e::escaped_jail);
      widget->addItem(tr("Flatter", "SM event name"), _e::flatter);
      widget->addItem(tr("Player Infected", "SM event name"), _e::player_infected);
      widget->addItem(tr("Intimidate", "SM event name"), _e::intimidate);
      widget->addItem(tr("Jail", "SM event name"), _e::jail);
      widget->addItem(tr("Kill", "SM event name"), _e::kill);
      widget->addItem(tr("Level Up", "SM event name"), _e::level_up);
      widget->addItem(tr("Lockpick", "SM event name"), _e::lockpick);
      widget->addItem(tr("New Voice Power", "SM event name"), _e::new_voice_power);
      widget->addItem(tr("Pay Fine", "SM event name"), _e::pay_fine);
      widget->addItem(tr("Player Receives Favor", "SM event name"), _e::player_receives_favor);
      widget->addItem(tr("Player Remove Item", "SM event name"), _e::player_remove_item);
      widget->addItem(tr("Quest Start", "SM event name"), _e::quest_start);
      widget->addItem(tr("Scripted Event", "SM event name"), _e::script);
      widget->addItem(tr("Skill Increase", "SM event name"), _e::skill_increase);
      widget->addItem(tr("Served Time in Jail", "SM event name"), _e::served_time_in_jail);
      widget->addItem(tr("Trespass", "SM event name"), _e::trespass);
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
      cobb::qt::bind(this->ui.eventType, working.event);
      this->ui.textDisplayGlobals->import(working.text_display_globals);
      //
      cobb::qt::bind(this->ui.priority, working.priority);
      this->ui.dialogueConditions->model()->setTarget(*this->stub, working.conditions.dialogue);
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
   auto& working = *this->form.ptr_cast<form_t>();
   //
   this->stub->editorID = this->ui.editorID->text().toStdString();
   editor.assign_localized_string(working.name, this->ui.name->text());
   this->ui.textDisplayGlobals->commit(working.text_display_globals, working);

   // TODO: EVERYTHING THAT DOESN'T MODIFY THE WORKING COPY IN REAL-TIME
}