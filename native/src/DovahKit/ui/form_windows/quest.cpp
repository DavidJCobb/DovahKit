#include "./quest.h"
#include <QWhatsThis> // for the "Help" button in the dialogue tab
#include "dovah/form_stub_addenda.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "./odds_and_ends/quest_tab_stages.h"
#include "./odds_and_ends/quest_tab_objectives.h"
#include "./quest/QuestTabAliases.h"
#include "./quest/QuestTabScenes.h"

#include "ui/utils/bind.h"

#include "./quest/QuestAllDialogueDatastore.h"
#include "./quest/QuestDialogueTabBody.h"

// focusing a scene's topic/infos
#include "dovah/forms/Scene.h"

FormDialogQuest::FormDialogQuest(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   initialize(stub);

   this->data.dialogue_datastore = new QuestAllDialogueDatastore(this);
   
   this->ui.textDisplayGlobals->setAllowedFormTypes({ dovah::form_type::global });
   {
      using _e = loaded_form_type::quest_type::type;
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
   this->load(); // this creates the working copy. we need that on hand before we create our tab-body widgets
   //
   this->ui.tabWidget->setCurrentIndex(0);
   this->ui.dialogueTabbox->setCurrentIndex(0);
   //
   #pragma region Create tabs
   {
      auto* tabbox  = this->ui.tabWidget;
      auto  _insert = [tabbox](int i, QWidget* body) {
         auto* page   = tabbox->widget(i);
         auto* layout = new QGridLayout(page);
         assert(page);
         layout->addWidget(body);
         page->setLayout(layout);
      };
      
      _insert(1, (this->tabs.stages     = new QuestTabStages(stub, *this->form, *this->ui.scriptListPane)));
      _insert(2, (this->tabs.objectives = new QuestTabObjectives(stub, *this->form)));
   }
   {
      auto* ds = this->data.dialogue_datastore;
      auto& dst = this->subwidgets.dialogue_tab_bodies;

      auto _make_tab = [this, ds](dovah::dialogue::category c, QWidget* container, QuestDialogueTabBody*& body_ptr) {
         auto* layout = new QGridLayout(container);

         body_ptr = new QuestDialogueTabBody(this);
         layout->addWidget(body_ptr);
         body_ptr->setCategory(c);
         body_ptr->setDatastore(ds);
      };

      _make_tab(
         dovah::dialogue::category::topic,
         this->ui.tabDialoguePlayer,
         dst.player
      );
      _make_tab(
         dovah::dialogue::category::favors,
         this->ui.tabDialogueFavor,
         dst.favor_b
      );
      _make_tab(
         dovah::dialogue::category::combat,
         this->ui.tabDialogueCombat,
         dst.combat
      );
      _make_tab(
         dovah::dialogue::category::detection,
         this->ui.tabDialogueDetection,
         dst.detection
      );
      _make_tab(
         dovah::dialogue::category::service,
         this->ui.tabDialogueService,
         dst.services
      );
      _make_tab(
         dovah::dialogue::category::miscellaneous,
         this->ui.tabDialogueMisc,
         dst.misc
      );
      _make_tab(
         dovah::dialogue::category::favor_dialogue,
         this->ui.tabDialogueFavorOld,
         dst.favor_a
      );
   }
   #pragma endregion
   
   {  // Aliases tab
      auto* manager = this->tabs.aliases = new QuestTabAliases(*this->form, this);
      manager->ui.view = this->ui.aliasList;
      manager->setupUi();
   }
   {  // Scene tab
      auto* manager = this->tabs.scenes = new QuestTabScenes(*this->form, *this->data.dialogue_datastore, this);
      manager->ui = {
         .buttons = {
            .zoom_in  = this->ui.buttonSceneZoomIn,
            .zoom_out = this->ui.buttonSceneZoomOut,
         },
         .show_all_text = this->ui.sceneShowAllText,

         .scene_picker = this->ui.scenePicker,

         .current_scene = {
            .buttons = {
               .actor_behavior      = this->ui.buttonSceneActorBehavior,
               .actor_participation = this->ui.buttonSceneActorParticipation,
            },
            .editor_id = this->ui.sceneEditorID,
            .flags = {
               .start_scene_with_quest = this->ui.flagStartSceneWithQuest,
               .end_quest_with_scene   = this->ui.flagEndQuestWithScene,
            },
            .editor_scrollbox = this->ui.sceneScrollbox,
            .editor = nullptr,
         },
      };
      manager->setupUi();
   }

   {  // "Help" button on dialogue tab
      auto* sub_tabbox = this->ui.dialogueTabbox;
      auto* button     = this->ui.buttonDialogueHelp;
      sub_tabbox->setCornerWidget(button, Qt::Corner::TopRightCorner);

      QObject::connect(button, &QPushButton::clicked, this, [this, button]() {
         QWhatsThis::showText(
            button->mapToGlobal(button->pos()),
            button->whatsThis(),
            button
         );
      });
   }
}
void FormDialogQuest::_load_impl() {
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;
   
   #pragma region Basic Data
      ui::bind(this->ui.editorID, this->editor_id());
      ui::bind(this->ui.editorCategory, working.filter);
      this->ui.name->setText(gls.convert_localized_string(working.name));
      ui::bind<loaded_form_type::quest_type::type>(this->ui.questType, working.quest_type);
      ui::bind(this->ui.flagAllowRepeatedStages,       working.flags, loaded_form_type::quest_flag::allow_repeated_stages);
      ui::bind(this->ui.flagExcludeFromDialogueExport, working.flags, loaded_form_type::quest_flag::exclude_from_dialogue_export);
      ui::bind(this->ui.flagRunOnce,                   working.flags, loaded_form_type::quest_flag::run_once);
      ui::bind(this->ui.flagStartGameEnabled,          working.flags, loaded_form_type::quest_flag::start_game_enabled);
      ui::bind(this->ui.flagWarnOnAliasFillFailure,    working.flags, loaded_form_type::quest_flag::warn_on_alias_fill_failure);
      ui::bind<dovah::story_event_code::type>(this->ui.eventType, working.event);
      this->ui.textDisplayGlobals->pullStubs(working.text_display_globals);
      
      ui::bind(this->ui.priority, working.priority);
      this->ui.dialogueConditions->importFrom(*this->form, working.conditions.dialogue);
   #pragma endregion
   #pragma region Stages
      //
      // Done by the subwidget, in its constructor.
      //
   #pragma endregion
   #pragma region Objectives
      //
      // Done by the subwidget, in its constructor.
      //
   #pragma endregion
   #pragma region Aliases
      //
      // Done by the subwidget, in its constructor.
      //
   #pragma endregion
   #pragma region Dialogue
      this->data.dialogue_datastore->set_quest(this->formStub());
   #pragma endregion
   #pragma region Scenes
      //
      // N/A
      //
   #pragma endregion
   #pragma region Scripts
      this->ui.scriptListPane->setFormWorkingCopy(&working);
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
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   {  // HACK to deal with event conditions
      //
      // When you edit Story Manager event trees, you'll find that you can put 
      // conditions on a Quest Node (SMQN), which is a container for quests, or 
      // on an individual Quest (QUST) in that node. The trick is that in the 
      // latter case, the conditions are stored on the QUST form itself. So this 
      // is form data in QUST that is only visible and alterable via the SM*N UI.
      // 
      // DovahKit's current design doesn't allow us to commit "everything except 
      // the event conditions;" I have rewrite plans in mind which would allow 
      // me to be that granular, but that isn't possible now. For now, we rely 
      // on the "form working copy" system, which means we're basically bulldozing 
      // *all* of the form data. So, we need to manually load the form data that 
      // we're going to overwrite, and copy the event conditions from the form 
      // data to our working copy.
      // 
      // Why not just leave the data unchanged in the working copy? Two reasons:
      // 
      //  - Working copies don't have their use info managed automatically. If a 
      //    form is deleted, any data in the working copy which refers to that 
      //    form will be left with a dangling use. Most of the time, our UI copes 
      //    with that by just manually managing things, or by managing things via 
      //    its widgets.
      // 
      //  - If you open the QUST UI, and then the SM*N UI, and then make changes 
      //    via the latter, those changes won't be visible to the QUST UI. Pulling 
      //    the conditions from the "canonical" form data ensures we won't end up 
      //    clobbering those changes.
      //
      auto& dst = working.conditions.event;
      dst.clear(working);

      auto src_ptr = this->stub->load().ptr_cast<loaded_form_type>();
      if (src_ptr) {
         auto& src = src_ptr->conditions.event;
         dst.append_all_of(working, src);
      }
   }
   
   #pragma region Basic Data
      gls.assign_localized_string(working.name, this->ui.name->text());
      this->ui.textDisplayGlobals->commitStubs(working.text_display_globals, working);
      this->ui.dialogueConditions->exportTo(*this->form, working.conditions.dialogue);
   #pragma endregion
   #pragma region Stages
      //
      // The subwidget makes all changes in real-time.
      //
   #pragma endregion
   #pragma region Objectives
      //
      // The subwidget makes all changes in real-time.
      //
   #pragma endregion
   #pragma region Aliases
      //
      // The subwidget makes all changes in real-time.
      //
   #pragma endregion
   #pragma region Dialogue
      //
      // The subwidgets make all changes in real-time.
      //
   #pragma endregion
   #pragma region Scenes
      this->tabs.scenes->push_data_to_scene_form(); // commit any pending changes for selected scene
   #pragma endregion
   #pragma region Scripts
      this->ui.scriptListPane->commit();
   #pragma endregion
}

#include <QApplication>
#include "dovah/data/dialogue/topic_subtype.h"
#include "dovah/form_stubs/helpers/for_each_quest_scene.h"
#include "dovah/form_stubs/helpers/get_dialogue_branch_quest.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_quest.h"
#include "dovah/forms/Topic.h"

void FormDialogQuest::focus_dialogue_branch(dovah::form_stub& stub) {
   auto* quest = dovah::form_stub_helpers::get_dialogue_branch_quest(stub);
   if (quest != this->formStub())
      return;

   this->ui.tabWidget->setCurrentWidget(this->ui.tabDialogue);
   this->ui.dialogueTabbox->setCurrentWidget(this->ui.tabDialoguePlayer);
   this->subwidgets.dialogue_tab_bodies.player->select_branch(&stub);
}
void FormDialogQuest::focus_dialogue_topic(dovah::form_stub& stub, dovah::form_stub* info) {
   auto* quest = dovah::form_stub_helpers::get_dialogue_topic_quest(stub);
   if (quest != this->formStub())
      return;

   dovah::dialogue::category cat;
   {
      auto loaded = stub.load().ptr_cast<dovah::loaded_forms::Topic>();
      if (!loaded)
         return;
      auto* info = dovah::dialogue::topic_subtype_by_signature(loaded->subtype);
      if (!info)
         return;
      cat = info->category;
   }

   QuestDialogueTabBody* browser = nullptr;
   QWidget* tab = nullptr;
   switch (cat) {
      case dovah::dialogue::category::topic:
         tab     = this->ui.tabDialoguePlayer;
         browser = this->subwidgets.dialogue_tab_bodies.player;
         break;
      case dovah::dialogue::category::scene:
         this->ui.tabWidget->setCurrentWidget(this->ui.tabScenes);
         {
            dovah::form_stub* topic = &stub;
            dovah::form_stub_helpers::for_each_quest_scene(*this->formStub(), [this, topic, &info](dovah::form_stub& scene) {
               bool refers_to_topic = false;
               for (auto& use : scene.outbound) {
                  if (use.second.other == topic) {
                     refers_to_topic = true;
                     break;
                  }
               }
               if (!refers_to_topic)
                  return false;

               auto loaded = scene.load().ptr_cast<dovah::loaded_forms::Scene>();
               if (!loaded)
                  return false;

               for (const auto& action : loaded->actions) {
                  auto* variant = std::get_if<dovah::loaded_forms::Scene::action::dialogue_data>(&action.data);
                  if (!variant)
                     continue;
                  if (variant->topic != topic)
                     continue;

                  this->tabs.scenes->select_scene(&scene);
                  this->tabs.scenes->focus_dialogue_forms(action.action_id, topic, info);
                  return true;
               }
               return false;
            });
         }
         return;
      case dovah::dialogue::category::favor_dialogue:
         tab     = this->ui.tabDialogueFavorOld;
         browser = this->subwidgets.dialogue_tab_bodies.favor_a;
         break;
      case dovah::dialogue::category::favors:
         tab     = this->ui.tabDialogueFavor;
         browser = this->subwidgets.dialogue_tab_bodies.favor_b;
         break;
      case dovah::dialogue::category::combat:
         tab     = this->ui.tabDialogueCombat;
         browser = this->subwidgets.dialogue_tab_bodies.combat;
         break;
      case dovah::dialogue::category::detection:
         tab     = this->ui.tabDialogueDetection;
         browser = this->subwidgets.dialogue_tab_bodies.detection;
         break;
      case dovah::dialogue::category::service:
         tab     = this->ui.tabDialogueService;
         browser = this->subwidgets.dialogue_tab_bodies.services;
         break;
      case dovah::dialogue::category::miscellaneous:
         tab     = this->ui.tabDialogueMisc;
         browser = this->subwidgets.dialogue_tab_bodies.misc;
         break;
   }
   if (!browser || !tab)
      return;

   this->ui.tabWidget->setCurrentWidget(this->ui.tabDialogue);
   this->ui.dialogueTabbox->setCurrentWidget(tab);

   if (info)
      browser->select_info(info);
   else
      browser->select_topic(&stub);
}
void FormDialogQuest::focus_dialogue_info(dovah::form_stub& stub) {
   if (stub.form_type != dovah::form_type::topic_info)
      return;
   auto* parent = stub.get_parent_form();
   if (!parent || parent->form_type != dovah::form_type::topic)
      return;
   this->focus_dialogue_topic(*parent, &stub);
}