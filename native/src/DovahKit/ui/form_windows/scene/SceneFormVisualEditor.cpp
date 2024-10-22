#include "./SceneFormVisualEditor.h"
#include <QPaintEvent>
#include <QPainter>
#include "dovah/form_stubs/helpers/get_unique_outbound_use.h"
#include "dovah/form_stub.h"
#include "dovah/forms/components/papyrus/fragment_data/scene_fragment_data.h"
#include "dovah/forms/Quest.h"
#include "dovah/forms/Scene.h"
#include "./SceneFormVisualEditor_impl/Actor.h"
#include "./SceneFormVisualEditor_impl/Phase.h"
#include "./SceneFormVisualEditor_impl/StyleOption.h"

#include "./SceneFormVisualEditor_impl/DialogueAction.h"
#include "./SceneFormVisualEditor_impl/PackageAction.h"
#include "./SceneFormVisualEditor_impl/TimerAction.h"

#include "ui/form_windows/quest/QuestAllDialogueDatastore.h"

#include <QBoxLayout>
#include <QDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>
#include "./SceneActorBehaviorModel.h"
#include "./SceneActorParticipationModel.h"
#include "./FormSubdialogScenePhase.h"
#include "./FormSubdialogSceneDialogueAction.h"
#include "./FormSubdialogScenePackageAction.h"
#include "./FormSubdialogSceneTimerAction.h"

namespace {
   constexpr const bool only_import_actions_belonging_to_defined_actors = true;
}

namespace {
   using namespace SceneFormVisualEditor_impl;
   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
   }

   using loaded_form_type = dovah::loaded_forms::Scene;

   constexpr const uint32_t no_alias = (uint32_t)-1;
}

SceneFormVisualEditor::SceneFormVisualEditor(QWidget* parent) : QWidget(parent) {
   this->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
   this->setSizePolicy({ QSizePolicy::Fixed, QSizePolicy::Fixed });

   auto palette = this->palette();
   this->_style.selection = {
      .background = palette.highlight().color(),
      .text       = palette.highlightedText().color(),
   };
   palette.setCurrentColorGroup(QPalette::ColorGroup::Inactive);
   this->_style.selection.inactive = {
      .background = palette.highlight().color(),
      .text       = palette.highlightedText().color(),
   };

   {
      auto& menu  = this->_context_menu.menu;
      auto& items = this->_context_menu;
      {
         auto* item = items.new_actor = new QAction(tr("Add actor..."), this);
         menu.addAction(item);
      }
      {
         auto& submenu = items.new_action;
         menu.addMenu(&submenu);
         submenu.setTitle(tr("Add action"));
         {
            auto* item = items.new_action_type.dialogue = new QAction(tr("Dialogue"), this);
            submenu.addAction(item);
         }
         {
            auto* item = items.new_action_type.package = new QAction(tr("Package"), this);
            submenu.addAction(item);
         }
         {
            auto* item = items.new_action_type.timer = new QAction(tr("Timer"), this);
            submenu.addAction(item);
         }
      }
      {
         auto& submenu = items.new_phase;
         menu.addMenu(&submenu);
         submenu.setTitle(tr("Add phase"));
         {
            auto* item = items.new_phase_where.before_here = new QAction(tr("Before this phase"), this);
            submenu.addAction(item);
         }
         {
            auto* item = items.new_phase_where.after_here = new QAction(tr("After this phase"), this);
            submenu.addAction(item);
         }
         {
            auto* item = items.new_phase_where.at_end = new QAction(tr("At end"), this);
            submenu.addAction(item);
         }
      }
      {
         auto* item = items.edit = new QAction(tr("Edit..."), this);
         menu.addAction(item);
      }
      {
         auto* item = items.remove = new QAction(tr("Delete"), this);
         menu.addAction(item);
      }
   }
}

SceneFormVisualEditor::~SceneFormVisualEditor() {
   this->_context = {};
   this->clear();
}

void SceneFormVisualEditor::setContext(const SceneContext& c) {
   this->_context = c;
   if (auto* stub = this->_context.quest) {
      if (stub->form_type != dovah::form_type::quest)
         this->_context.quest = nullptr;
   }
   if (auto* stub = this->_context.scene) {
      if (stub->form_type != dovah::form_type::scene) {
         this->_context.scene = nullptr;
      } else if (!this->_context.quest) {
         this->_context.quest = dovah::form_stub_helpers::get_unique_outbound_use<dovah::use_info_entry::flag::dialogue_quest>(*stub);
      }
   }
   this->_update_phase_conditions();
}

bool SceneFormVisualEditor::isReferenceAliasUsed(uint32_t id) const {
   for (const auto* item : this->_data.actors)
      if (item->alias_id == id)
         return true;
   for (const auto* item : this->_data.actions) {
      if (item->base_data.alias_id == id) {
         #if _DEBUG
            if constexpr (only_import_actions_belonging_to_defined_actors) {
               __debugbreak(); // This action shouldn't be here!
            }
         #endif
         return true;
      }
      if (const auto* casted = dynamic_cast<const DialogueAction*>(item))
         if (casted->data.headtrack.alias_id == id)
            return true;
   }
   return false;
}
QString SceneFormVisualEditor::referenceAliasName(uint32_t id) const {
   if (auto* stub = this->_context.quest) {
      dovah::loaded_forms::Alias* alias = nullptr;
      if (auto* form = (dovah::loaded_forms::Quest*) stub->get_working_copy()) {
         alias = form->lookup_alias_by_id(id);
      } else {
         auto loaded = stub->load().ptr_cast<dovah::loaded_forms::Quest>();
         if (loaded)
            alias = loaded->lookup_alias_by_id(id);
      }
      if (alias)
         return QString::fromStdString(alias->name);
   }
   return "";
}

void SceneFormVisualEditor::importData() {
   this->_clear_data();

   if (!this->_context.scene)
      return;
   const auto* working = this->_context.scene->get_working_copy();
   if (!working)
      return;
   const auto& form = *(loaded_form_type*)working;

   const vmad::scene_fragment_data* fragment_data = nullptr;
   if (const auto* src_frags = form.script_data.fragment_data) {
      if (src_frags->type == vmad::fragment_type::scene) {
         fragment_data = (const vmad::scene_fragment_data*) src_frags;
      }
   }

   {  // Actors
      using src_type = loaded_form_type::actor;

      this->_data.actors.reserve(form.actors.size());
      for (auto& src : form.actors) {
         if (src.alias_id == no_alias)
            continue;

         auto* dst = new Actor;
         this->_data.actors.push_back(dst);
         dst->alias_id = src.alias_id;
         {
            auto  src_flags = src.behavior_flags;
            auto& dst_flags = dst->behavior_flags;

            auto _write_flag = [](ActorBehaviorFlags& dst, int src) {
               dst = {};
               if (src & 1)
                  dst.pause = true;
               if (src & 2)
                  dst.end = true;
            };
            _write_flag(dst_flags.death, src_flags & 0b11);
            _write_flag(dst_flags.combat, (src_flags >> 2) & 0b11);
            _write_flag(dst_flags.dialogue, (src_flags >> 4) & 0b11);
            _write_flag(dst_flags.observe_corpse, (src_flags >> 8) & 0b11);
         }
         dst->participation_flags.no_player_activation = src.participation_flags & src_type::participation_flag::no_player_activation;
         dst->participation_flags.optional = src.participation_flags & src_type::participation_flag::optional;
      }
   }
   {  // Phases
      using src_type = loaded_form_type::phase;

      this->_data.phases.reserve(form.phases.size());
      for (auto& src : form.phases) {
         auto* dst = new Phase;
         this->_data.phases.push_back(dst);

         dst->data.importMainData(src);
         dst->editor_width = src.editor_display_width;
      }
      if (fragment_data) {
         using phase_fragment = vmad::scene_fragment_data::phase_fragment;

         for (auto& f : fragment_data->fragments.on_phase) {
            if (f.phase < this->_data.phases.size()) {
               auto& dst = *this->_data.phases[f.phase];
               dst.data.importFragments(f);
            }
         }
      }
   }
   {  // Actions
      using src_type = loaded_form_type::action;

      this->_data.actions.reserve(form.actions.size());
      for (auto& src : form.actions) {
         if (src.alias_id == no_alias)
            continue;
         if constexpr (only_import_actions_belonging_to_defined_actors) {
            bool used = false;
            for (auto* item : this->_data.actors) {
               if (item->alias_id == src.alias_id) {
                  used = true;
                  break;
               }
            }
            if (!used)
               continue;
         }

         Action* dst = nullptr;
         if (auto* casted = std::get_if<src_type::dialogue_data>(&src.data)) {
            auto* casted_dst = new DialogueAction;
            dst = casted_dst;

            casted_dst->data.topic = casted->topic.get_form_stub();
            casted_dst->data.headtrack.alias_id    = casted->headtrack_alias_id;
            casted_dst->data.headtrack.at_player   = src.flags & src_type::flag::headtrack_player;
            casted_dst->data.headtrack.face_target = src.flags & src_type::flag::face_target;
            casted_dst->data.emotion.type  = casted->emotion.type;
            casted_dst->data.emotion.value = casted->emotion.value;
            casted_dst->data.looping.min = casted->looping.min;
            casted_dst->data.looping.max = casted->looping.max;
         } else if (auto* casted = std::get_if<src_type::package_data>(&src.data)) {
            auto* casted_dst = new PackageAction;
            dst = casted_dst;

            casted_dst->data.packages.reserve(casted->packages.size());
            for (auto& use : casted->packages)
               if (use)
                  casted_dst->data.packages.push_back(use.get_form_stub());
         } else if (auto* casted = std::get_if<src_type::timer_data>(&src.data)) {
            auto* casted_dst = new TimerAction;
            dst = casted_dst;

            casted_dst->data.duration = casted->duration;
         }
         this->_data.actions.push_back(dst);
         dst->base_data.name      = QString::fromStdString(src.name);
         dst->base_data.alias_id  = src.alias_id;
         dst->base_data.action_id = src.action_id;
         dst->base_data.phase_indices.start = src.phase_indices.start;
         dst->base_data.phase_indices.end   = src.phase_indices.end;
      }
      if (fragment_data) {
         using phase_fragment = vmad::scene_fragment_data::phase_fragment;

         for (auto& f : fragment_data->fragments.on_phase) {
            if (f.flags & phase_fragment::flag::on_start)
               continue;
            if (f.flags & phase_fragment::flag::on_completion)
               continue;
            //
            // This is an action fragment.
            //
            TimerAction* action = nullptr;
            for (auto* item : this->_data.actions) {
               if (item->base_data.action_id != f.phase)
                  continue;
               action = dynamic_cast<TimerAction*>(item);
               break;
            }
            if (action) {
               action->data.fragment = {
                  .scriptname = f.filename,
                  .function   = f.function,
               };
            }
         }
      }
   }
   this->_update_cached_internal_relationships();
   {  // Cache all relevant form info (editor IDs, etc.)
      for (auto* item : this->_data.actors)
         item->cached.alias_name = this->referenceAliasName(item->alias_id);

      this->_update_phase_conditions(false);

      for (auto* action : this->_data.actions) {
         if (auto* casted = dynamic_cast<DialogueAction*>(action)) {
            auto& dst = casted->cached.infos;
            dst.clear();

            if (!this->_context.dialogue)
               continue;
            if (!casted->data.topic)
               continue;
            auto* data = this->_context.dialogue->item_for_topic_stub(*casted->data.topic);
            if (!data)
               continue;

            for (auto* info : data->infos) {
               dst.push_back(info->cached.responses);
            }
         }
      }
   }
   this->_update_geometry();
   this->updateGeometry();
}
void SceneFormVisualEditor::exportData() {
   if (!this->_context.scene)
      return;
   auto* working = this->_context.scene->get_working_copy();
   if (!working)
      return;
   auto& form = *(loaded_form_type*)working;

   vmad::scene_fragment_data* dst_frags = nullptr;
   {  // Papyrus
      auto* dst_frags_bare = form.script_data.fragment_data;
      if (dst_frags_bare) {
         if (dst_frags_bare->type != vmad::fragment_type::scene) {
            dst_frags_bare->clear(form);
            delete dst_frags_bare;
            dst_frags_bare = nullptr;
         }
      }
      if (!dst_frags_bare) {
         dst_frags_bare = form.script_data.fragment_data = new vmad::scene_fragment_data;
      }
      dst_frags = (vmad::scene_fragment_data*) dst_frags_bare;
   }

   {  // Actors
      using dst_type = loaded_form_type::actor;

      form.actors.clear();
      for (auto* src : this->_data.actors) {
         auto& dst = form.actors.emplace_back();
         dst.alias_id = src->alias_id;
         dst.behavior_flags = 0;
         dst.participation_flags = 0;

         auto _write_flag = [&dst](ActorBehaviorFlags& src, int nth) {
            dst.behavior_flags &= ~(0 << (nth * 2));
            if (src.pause)
               dst.behavior_flags |= 1 << (nth * 2);
            if (src.end)
               dst.behavior_flags |= 2 << (nth * 2);
         };
         _write_flag(src->behavior_flags.death, 0);
         _write_flag(src->behavior_flags.combat, 1);
         _write_flag(src->behavior_flags.dialogue, 2);
         _write_flag(src->behavior_flags.observe_corpse, 3);

         if (src->participation_flags.no_player_activation)
            dst.participation_flags |= dst_type::participation_flag::no_player_activation;
         if (src->participation_flags.optional)
            dst.participation_flags |= dst_type::participation_flag::optional;
      }
   }
   {  // Phases
      auto _copy_conditions = [&form](
         std::vector<ui::types::conditions::condition>& src_list,
         dovah::loaded_forms::components::condition_list& dst_list
         ) {
         dst_list.clear(form);
         dst_list.append_all_of(form, src_list);
      };

      auto& src_list = this->_data.phases;
      auto& dst_list = form.phases;
      {
         for (auto& item : dst_list)
            item.clear(form);
         dst_list.clear();
      }
      size_t size = src_list.size();
      dst_list.resize(size);
      for (size_t i = 0; i < size; ++i) {
         assert(src_list[i] != nullptr);
         auto& src_item = *src_list[i];
         auto& dst_item = dst_list[i];
         dst_item.name = src_item.data.name.toStdString();
         _copy_conditions(src_item.data.conditions.start, dst_item.conditions.start);
         _copy_conditions(src_item.data.conditions.completion, dst_item.conditions.completion);
         dst_item.editor_display_width = src_item.editor_width;
      }
      {
         dst_frags->fragments.on_phase.clear();
         for (size_t i = 0; i < size; ++i) {
            const auto& phase = *src_list[i];
            {
               auto& src = phase.data.fragments.start;
               if (!src.scriptname.empty() || !src.function.empty()) {
                  auto& frag = dst_frags->fragments.on_phase.emplace_back();
                  frag.phase    = i;
                  frag.flags    = vmad::scene_fragment_data::phase_fragment::flag::on_start;
                  frag.filename = src.scriptname;
                  frag.function = src.function;
               }
            }
            {
               auto& src = phase.data.fragments.completion;
               if (!src.scriptname.empty() || !src.function.empty()) {
                  auto& frag = dst_frags->fragments.on_phase.emplace_back();
                  frag.phase    = i;
                  frag.flags    = vmad::scene_fragment_data::phase_fragment::flag::on_completion;
                  frag.filename = src.scriptname;
                  frag.function = src.function;
               }
            }
         }
      }
   }
   {  // Actions
      using dst_type = loaded_form_type::action;

      auto& src_list = this->_data.actions;
      auto& dst_list = form.actions;
      {
         for (auto& item : dst_list)
            item.clear(form);
         dst_list.clear();
      }
      size_t size = src_list.size();
      dst_list.resize(size);
      for (size_t i = 0; i < size; ++i) {
         assert(src_list[i] != nullptr);
         auto& src_item = *src_list[i];
         auto& dst_item = dst_list[i];
         dst_item.name      = src_item.base_data.name.toStdString();
         dst_item.alias_id  = src_item.base_data.alias_id;
         dst_item.action_id = src_item.base_data.action_id;
         dst_item.flags &= ~(
            dst_type::flag::face_target |
            dst_type::flag::headtrack_player |
            dst_type::flag::looping
         );
         dst_item.phase_indices.start = src_item.base_data.phase_indices.start;
         dst_item.phase_indices.end   = src_item.base_data.phase_indices.end;
         if (auto* casted = dynamic_cast<DialogueAction*>(&src_item)) {
            auto& dst_data = dst_item.data.emplace<dst_type::dialogue_data>();
            dst_data.topic.set(form, casted->data.topic);
            dst_data.emotion.type  = casted->data.emotion.type;
            dst_data.emotion.value = casted->data.emotion.value;
            dst_data.looping.min = casted->data.looping.min;
            dst_data.looping.max = casted->data.looping.max;
            dst_data.headtrack_alias_id = casted->data.headtrack.alias_id;
            {
               if (casted->data.headtrack.face_target)
                  dst_item.flags |= dst_type::flag::face_target;
               if (casted->data.headtrack.at_player)
                  dst_item.flags |= dst_type::flag::headtrack_player;
               if (casted->data.looping.enabled)
                  dst_item.flags |= dst_type::flag::looping;
            }
         } else if (auto* casted = dynamic_cast<PackageAction*>(&src_item)) {
            auto& dst_data = dst_item.data.emplace<dst_type::package_data>();
            {
               auto& dst_list = dst_data.packages;
               for (auto* item : casted->data.packages) {
                  if (!item)
                     continue;
                  dst_list.emplace_back().set(form, item);
               }
            }
         } else if (auto* casted = dynamic_cast<TimerAction*>(&src_item)) {
            auto& dst_data = dst_item.data.emplace<dst_type::timer_data>();
            dst_data.duration = casted->data.duration;

            if (!casted->data.fragment.empty()) {
               auto& frag = dst_frags->fragments.on_phase.emplace_back();
               frag.phase    = casted->base_data.action_id;
               frag.flags    = 0;
               frag.filename = casted->data.fragment.scriptname;
               frag.function = casted->data.fragment.function;
            }
         }
      }
   }
}

void SceneFormVisualEditor::focus_dialogue_forms(uint32_t action_id, dovah::form_stub* topic, dovah::form_stub* info) {
   if (!this->_context.dialogue)
      return;

   DialogueAction* casted = nullptr;
   FormSubdialogSceneDialogueAction* dialog = nullptr;
   for (auto* action : this->_data.actions) {
      if (action->base_data.action_id != action_id)
         continue;
      casted = dynamic_cast<DialogueAction*>(action);
      if (!casted)
         continue;

      dialog = new FormSubdialogSceneDialogueAction(*this->_context.dialogue, this);
      break;
   }
   if (!dialog)
      return;

   this->_set_up_action_dialog_phases(*casted, *dialog);
   for (auto* actor : this->_data.actors) {
      dialog->scene_data.actors.push_back({ actor->alias_id, actor->cached.alias_name });
   }
   
   dialog->base_data = casted->base_data;
   dialog->data      = casted->data;
   dialog->refresh();
   dialog->show_info_on_open = info;
   if (dialog->exec() == QDialog::Accepted) {
      this->_data.any_changes_made = true;
      casted->base_data = dialog->base_data;
      casted->data      = dialog->data;
      this->_update_cached_internal_relationships();
      this->_update_geometry();
      this->update();
   }
   dialog->deleteLater();
}

void SceneFormVisualEditor::popBehaviorFlagDialog() {
   auto* dialog = new QDialog(this);
   auto* layout = new QGridLayout(dialog);
   auto* view   = new QTableView(dialog);
   auto* model  = new SceneActorBehaviorModel(view);
   view->setModel(model);
   {
      dialog->setWindowTitle(tr("Scene Actor Events"));
      layout->addWidget(view, 0, 0);

      auto* nested = new QHBoxLayout(dialog);
      layout->addLayout(nested, 1, 0);
      nested->addStretch(1);
      {
         auto* button = new QPushButton(tr("OK"), dialog);
         nested->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, dialog, &QDialog::accept);
      }
      {
         auto* button = new QPushButton(tr("Cancel"), dialog);
         nested->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, dialog, &QDialog::reject);
      }
      nested->addStretch(1);
   }
   for (const auto* actor : this->_data.actors) {
      model->addActor(*actor);
   }
   if (dialog->exec() == QDialog::Accepted) {
      this->_data.any_changes_made = true;
      for (auto* actor : this->_data.actors) {
         model->commitActor(*actor);
      }
      //
      // This data is displayed in the visual editor.
      //
      this->_update_geometry();
      this->update();
   }
   dialog->deleteLater();
}
void SceneFormVisualEditor::popParticipationFlagDialog() {
   auto* dialog = new QDialog(this);
   auto* layout = new QGridLayout(dialog);
   auto* view   = new QTableView(dialog);
   auto* model  = new SceneActorParticipationModel(view);
   view->setModel(model);
   {
      dialog->setWindowTitle(tr("Scene Actor Properties"));
      layout->addWidget(view, 0, 0);

      auto* nested = new QHBoxLayout(dialog);
      layout->addLayout(nested, 1, 0);
      nested->addStretch(1);
      {
         auto* button = new QPushButton(tr("OK"), dialog);
         nested->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, dialog, &QDialog::accept);
      }
      {
         auto* button = new QPushButton(tr("Cancel"), dialog);
         nested->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, dialog, &QDialog::reject);
      }
      nested->addStretch(1);
   }
   for (const auto* actor : this->_data.actors) {
      model->addActor(*actor);
   }
   if (dialog->exec() == QDialog::Accepted) {
      this->_data.any_changes_made = true;
      for (auto* actor : this->_data.actors) {
         model->commitActor(*actor);
      }
   }
   dialog->deleteLater();
}

void SceneFormVisualEditor::_clear_data() {
   {
      auto& list = this->_data.phases;
      for (auto* item : list)
         delete item;
      list.clear();
   }
   {
      auto& list = this->_data.actors;
      for (auto* item : list)
         delete item;
      list.clear();
   }
   {
      auto& list = this->_data.actions;
      for (auto* item : list)
         delete item;
      list.clear();
   }
   this->_data.any_changes_made = false;
}
void SceneFormVisualEditor::clear() {
   this->_context = {};
   this->_clear_data();
}
void SceneFormVisualEditor::spawnRenderTest() {
   this->clear();
   //
   // Test-case: MQ201PartyErikurIntroScene
   //
   for (size_t i = 1; i <= 11; ++i) {
      this->_data.phases.emplace_back() = new Phase;
      auto* phase = this->_data.phases.back();
      phase->editor_width = 200;
   }
   this->_data.phases[1]->cached.conditions.start = "(IsInDialogueWithPlayer NONE == 0.00)";
   this->_data.phases[7]->data.name = "Ready for Erikur distraction start (s)";
   this->_data.phases[10]->cached.conditions.completion = "(IsSceneActionComplete MQ201PartyErikurIntroScene, 13 == 1.00)";

   constexpr const auto pausing_aliases = std::array{
      std::pair<const char*, uint32_t>{ "Erikur", 58 },
      std::pair<const char*, uint32_t>{ "Brelas", 94 },
   };
   for (auto& pair : pausing_aliases) {
      this->_data.actors.emplace_back() = new Actor;
      auto* actor = this->_data.actors.back();
      actor->alias_id = pair.second;
      actor->cached.alias_name = pair.first;
      actor->behavior_flags.dialogue.pause = true;
   }
   //
   for (const char* name : { "Elenwen", "Razelan", "Ondolemar", "GeneralTullius", "VittoriaVici", "OrthusEndario", "ProventusAvenicci", "Maven" }) {
      this->_data.actors.emplace_back() = new Actor;
      auto* actor = this->_data.actors.back();
      actor->cached.alias_name = name;
   }

   auto& list = this->_data.actions;
   #pragma region Erikur
   {
      auto* action = new DialogueAction;
      this->_data.actions.push_back(action);
      action->base_data.action_id = 15;
      action->base_data.alias_id  = pausing_aliases[0].second;
      action->base_data.phase_indices = { 2, 2 };
   }
   {
      auto* action = new DialogueAction;
      this->_data.actions.push_back(action);
      action->base_data.action_id = 5;
      action->base_data.alias_id  = pausing_aliases[0].second;
      action->base_data.phase_indices = { 3, 3 };

      action->cached.infos.push_back("There's a likely-looking filly. Even if she is an elf.");
   }
   {
      auto* action = new DialogueAction;
      this->_data.actions.push_back(action);
      action->base_data.action_id = 6;
      action->base_data.alias_id  = pausing_aliases[0].second;
      action->base_data.phase_indices = { 4, 4 };

      action->cached.infos.push_back("You there! Serving girl! What's your name, dear?");
   }
   {
      auto* action = new DialogueAction;
      this->_data.actions.push_back(action);
      action->base_data.action_id = 8;
      action->base_data.alias_id  = pausing_aliases[0].second;
      action->base_data.phase_indices = { 6, 6 };

      action->cached.infos.push_back("No, no, that's not what I'm interested in right now. | I just wanted to get a better look at you. I like what I see, my dear. | And believe me, I don't say that to everyone. I'm very discriminating when it comes to the female form.");
   }
   {
      auto* action = new DialogueAction;
      this->_data.actions.push_back(action);
      action->base_data.action_id = 12;
      action->base_data.alias_id  = pausing_aliases[0].second;
      action->base_data.phase_indices = { 8, 8 };

      action->cached.infos.push_back("Oh... not at the moment. Maybe later. Don't go far.");
   }
   {
      auto* action = new PackageAction;
      this->_data.actions.push_back(action);
      action->base_data.action_id = 30;
      action->base_data.alias_id  = pausing_aliases[0].second;
      action->base_data.phase_indices = { 0, 3 };

      action->data.packages.push_back(nullptr);
      action->cached.package_editor_ids.push_back("MQ201ErikurApproachBrelas");
   }
   {
      auto* action = new PackageAction;
      this->_data.actions.push_back(action);
      action->base_data.action_id = 2;
      action->base_data.alias_id  = pausing_aliases[0].second;
      action->base_data.phase_indices = { 4, 10 };

      action->data.packages.push_back(nullptr);
      action->cached.package_editor_ids.push_back("DefaultStayAtCurrentLocationScene");
   }
   {
      auto* action = new TimerAction;
      this->_data.actions.push_back(action);
      action->base_data.action_id = 16;
      action->base_data.alias_id  = pausing_aliases[0].second;
      action->base_data.phase_indices = { 2, 2 };

      action->data.duration = 1.0F;
   }
   #pragma endregion

   this->_update_cached_internal_relationships();
   this->_update_geometry();
   this->updateGeometry();
}

void SceneFormVisualEditor::_set_up_action_dialog_phases(const Action& action, FormSubdialogSceneActionBase& dialog) {
   Actor* actor = nullptr;
   for (auto* item : this->_data.actors) {
      if (item->alias_id == action.base_data.alias_id) {
         actor = item;
         break;
      }
   }

   auto _phase_is_taken = [actor, &action](size_t i) -> bool {
      if (!actor)
         return false;
      for (auto* item : actor->cached.actions) {
         if (item == &action)
            continue;
         if (typeid(*item) != typeid(action))
            continue;
         auto& pi = item->base_data.phase_indices;
         if (pi.start <= i && pi.end >= i)
            return true;
      }
      return false;
   };

   for (size_t i = 0; i < this->_data.phases.size(); ++i) {
      if (_phase_is_taken(i))
         continue;
      auto* phase = this->_data.phases[i];
      auto  name  = phase->data.name;
      if (name.isEmpty()) {
         name = tr("Phase %1").arg(i + 1);
      }
      dialog.scene_data.phases.push_back({ i, name });
   }
}

void SceneFormVisualEditor::addActor() {
   auto* dialog = new QDialog(this);
   auto* layout = new QVBoxLayout(dialog);
   auto* model  = new QStandardItemModel;
   auto* view   = new QTableView(dialog);
   view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
   view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
   view->setModel(model);
   view->verticalHeader()->setVisible(false);
   view->verticalHeader()->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
   {
      dialog->setWindowTitle(tr("Add Scene Actor"));
      layout->addWidget(view);

      auto* nested = new QHBoxLayout(dialog);
      layout->addLayout(nested);
      nested->addStretch(1);
      {
         auto* button = new QPushButton(tr("OK"), dialog);
         nested->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, dialog, &QDialog::accept);
      }
      {
         auto* button = new QPushButton(tr("Cancel"), dialog);
         nested->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, dialog, &QDialog::reject);
      }
      nested->addStretch(1);
   }
   {  // populate dialog
      model->setColumnCount(1);
      model->setHeaderData(0, Qt::Horizontal, tr("Reference Alias"), Qt::DisplayRole);

      auto* stub = this->_context.quest;
      if (stub) {
         auto* form = (dovah::loaded_forms::Quest*) stub->get_working_copy();
         if (form) {
            for (auto* alias : form->aliases) {
               if (alias->type != dovah::loaded_forms::Alias::alias_type::reference)
                  continue;

               bool present = false;
               for (auto* actor : this->_data.actors) {
                  if (actor->alias_id == alias->id) {
                     present = true;
                     break;
                  }
               }
               if (!present) {
                  auto  name = QString::fromStdString(alias->name);
                  auto* item = new QStandardItem(name);
                  item->setData(name,      Qt::ToolTipRole);
                  item->setData(alias->id, Qt::UserRole);
                  item->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
                  model->appendRow(item);
               }
            }
         }
      }
   }
   if (dialog->exec() == QDialog::Accepted) {
      this->_data.any_changes_made = true;
      auto* sel  = view->selectionModel();
      auto  rows = sel->selectedRows();
      if (!rows.empty()) {
         auto qmi = rows[0];
         auto name     = model->data(qmi, Qt::DisplayRole).toString();
         auto alias_id = model->data(qmi, Qt::UserRole).toInt();

         auto* actor = new Actor;
         this->_data.actors.push_back(actor);
         actor->alias_id = alias_id;
         actor->cached.alias_name = name;

         this->_update_cached_internal_relationships();
         this->_update_geometry();
         this->update();
      }
   }
   dialog->deleteLater();

}
void SceneFormVisualEditor::editAction(Action& action) {
   auto* action_d = dynamic_cast<DialogueAction*>(&action);
   auto* action_p = dynamic_cast<PackageAction*>(&action);
   auto* action_t = dynamic_cast<TimerAction*>(&action);

   FormSubdialogSceneActionBase* untyped_dialog = nullptr;
   if (action_d) {
      if (!this->_context.dialogue)
         return;
      auto* dialog = new FormSubdialogSceneDialogueAction(*this->_context.dialogue, this);
      untyped_dialog = dialog;

      dialog->data = action_d->data;
   } else if (action_p) {
      auto* dialog = new FormSubdialogScenePackageAction(this);
      untyped_dialog = dialog;

      dialog->data = action_p->data;
   } else if (action_t) {
      auto* form    = (loaded_form_type*)this->_context.scene->get_working_copy();
      assert(form != nullptr);
      auto& working = *form;

      auto* dialog = new FormSubdialogSceneTimerAction(working, this);
      untyped_dialog = dialog;

      dialog->data = action_t->data;
   } else {
      return;
   }
   assert(untyped_dialog != nullptr);
   
   this->_set_up_action_dialog_phases(action, *untyped_dialog);
   for (auto* actor : this->_data.actors) {
      untyped_dialog->scene_data.actors.push_back({ actor->alias_id, actor->cached.alias_name });
   }

   untyped_dialog->base_data = action.base_data;
   untyped_dialog->refresh();
   if (untyped_dialog->exec() == QDialog::Accepted) {
      this->_data.any_changes_made = true;
      action.base_data = untyped_dialog->base_data;
      if (action_d) {
         auto& src = ((FormSubdialogSceneDialogueAction*)untyped_dialog)->data;
         action_d->data = src;
      } else if (action_p) {
         auto& src = ((FormSubdialogScenePackageAction*)untyped_dialog)->data;
         action_p->data = src;
      } else if (action_t) {
         auto& src = ((FormSubdialogSceneTimerAction*)untyped_dialog)->data;
         action_t->data = src;
      }
      this->_update_cached_internal_relationships();
      this->_update_geometry();
      this->update();
   }
   untyped_dialog->deleteLater();
}
void SceneFormVisualEditor::editPhase(Phase& phase) {
   auto* form    = (loaded_form_type*)this->_context.scene->get_working_copy();
   assert(form != nullptr);
   auto& working = *form;
   auto* dialog  = new FormSubdialogScenePhase(working, this);

   dialog->data = phase.data;
   dialog->refresh();

   if (dialog->exec() == QDialog::Accepted) {
      this->_data.any_changes_made = true;
      phase.data = dialog->data;
      //
      // Name or conditions may have changed; re-render.
      //
      this->_update_geometry();
      this->updateGeometry();
      this->update();
   }
   dialog->deleteLater();
}
void SceneFormVisualEditor::insertAction(Action& action, Actor& actor, Phase& phase) {
   size_t phase_index = -1;
   for (size_t i = 0; i < this->_data.phases.size(); ++i) {
      if (this->_data.phases[i] == &phase) {
         phase_index = i;
         break;
      }
   }
   assert(phase_index != -1);

   action.base_data.phase_indices.start = action.base_data.phase_indices.end = phase_index;
   action.base_data.alias_id = actor.alias_id;
   {
      uint32_t action_id = 0;
      for (auto* action : this->_data.actions) {
         action_id = std::max(action_id, action->base_data.action_id + 1);
      }
      action.base_data.action_id = action_id;
   }
   this->_data.actions.push_back(&action);

   this->_data.any_changes_made = true;
   this->_update_cached_internal_relationships();
   this->_update_geometry();
   this->update();
}
void SceneFormVisualEditor::insertPhaseAt(size_t at) {
   {
      auto& list = this->_data.phases;
      if (at > list.size())
         at = list.size();
      this->_data.phases.insert(list.begin() + at, new Phase);
   }
   for (auto* action : this->_data.actions) {
      auto& pi = action->base_data.phase_indices;
      if (pi.start >= at)
         ++pi.start;
      if (pi.end >= at)
         ++pi.end;
   }
   this->_data.any_changes_made = true;
   this->_update_cached_internal_relationships();
   this->_update_geometry();
   this->update();
}
void SceneFormVisualEditor::removeActor(uint32_t id) {
   bool needs_geo_update = false;

   needs_geo_update = std::erase_if(this->_data.actors, [id](Actor* a) { return a->alias_id == id; }) != 0;

   auto&  list = this->_data.actions;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* action = list[i];
      if (action->base_data.alias_id == id) {
         delete action;
         list.erase(list.begin() + i);
         --i;
         --size;
         needs_geo_update = true;
         continue;
      }
      if (auto* casted = dynamic_cast<DialogueAction*>(action)) {
         if (casted->data.headtrack.alias_id == id)
            casted->data.headtrack.alias_id = no_alias;
      }
   }

   this->_data.any_changes_made = true;
   if (needs_geo_update) {
      this->_update_geometry();
      this->update();
   }
}
void SceneFormVisualEditor::removeAction(Action& action) {
   std::erase(this->_data.actions, &action);
   delete &action;

   this->_data.any_changes_made = true;
   this->_update_cached_internal_relationships();
   this->_update_geometry();
   this->update();
}
void SceneFormVisualEditor::removePhase(Phase& phase) {
   size_t i = 0;
   for (; i < this->_data.phases.size(); ++i)
      if (this->_data.phases[i] == &phase)
         break;
   assert(i < this->_data.phases.size());

   auto&  list = this->_data.actions;
   size_t size = list.size();
   for (size_t j = 0; j < size; ++j) {
      auto* action = list[j];
      auto& pi     = action->base_data.phase_indices;
      if (pi.start == i && pi.end == i) {
         list.erase(list.begin() + j);
         delete action;
         --j;
         --size;
         continue;
      }
      if (pi.start >= i)
         --pi.start;
      if (pi.end >= i)
         --pi.end;
   }

   this->_data.any_changes_made = true;
   this->_update_cached_internal_relationships();
   this->_update_geometry();
   this->update();
}

#pragma region Overrides
   /*virtual*/ QSize SceneFormVisualEditor::minimumSizeHint() const /*override*/ {
      return this->sizeHint();
   }
   /*virtual*/ QSize SceneFormVisualEditor::sizeHint() const /*override*/ {
      return this->_cached.size;
   }

   /*virtual*/ void SceneFormVisualEditor::contextMenuEvent(QContextMenuEvent* event) /*override*/ {
      auto& menu  = this->_context_menu.menu;
      auto& items = this->_context_menu;

      struct {
         Phase* pointer   = nullptr;
         size_t index     = 0;
         bool   on_header = false;
      } clicked_phase;
      Actor*  clicked_actor  = nullptr;
      Action* clicked_action = nullptr;

      auto pos = event->pos();
      for (size_t i = 0; i < this->_data.phases.size(); ++i) {
         auto* item = this->_data.phases[i];
         const auto& rect = item->geometry.rect;
         if (pos.x() >= rect.left() && pos.x() <= rect.right()) {
            clicked_phase.pointer = item;
            clicked_phase.index   = i;
            //
            auto rect = item->geometry.rel.header;
            rect.moveTo(item->geometry.rect.topLeft());
            if (rect.contains(pos)) {
               clicked_phase.on_header = true;
            }
            //
            break;
         }
      }

      if (!clicked_phase.on_header && clicked_phase.pointer) {
         for (auto* item : this->_data.actors) {
            if (!item->geometry.rect.contains(pos))
               continue;
            clicked_actor = item;
         }
         if (clicked_actor) {
            for (auto* item : clicked_actor->cached.actions) {
               if (!item->geometry.rect.contains(pos))
                  continue;
               clicked_action = item;
               break;
            }
         }
      }

      bool has_selection = !std::holds_alternative<std::monostate>(this->_selection);
      items.edit->setEnabled(has_selection);
      items.remove->setEnabled(has_selection);

      items.new_phase_where.before_here->setEnabled(clicked_phase.pointer != nullptr);
      items.new_phase_where.after_here->setEnabled(clicked_phase.pointer != nullptr);
      if (clicked_actor) {
         bool has_dialogue = false;
         bool has_package  = false;
         bool has_timer    = false;
         for (const auto* action : clicked_actor->cached.actions) {
            if (action->base_data.phase_indices.end < clicked_phase.index)
               continue;
            if (action->base_data.phase_indices.start > clicked_phase.index)
               continue;

            if (dynamic_cast<const DialogueAction*>(action)) {
               has_dialogue = true;
            } else if (dynamic_cast<const PackageAction*>(action)) {
               has_package = true;
            } else if (dynamic_cast<const TimerAction*>(action)) {
               has_timer = true;
            }
         }
         items.new_action.setEnabled(has_dialogue || has_package || has_timer);
         items.new_action_type.dialogue->setEnabled(!has_dialogue);
         items.new_action_type.package->setEnabled(!has_package);
         items.new_action_type.timer->setEnabled(!has_timer);
      } else {
         items.new_action.setEnabled(false);
         items.new_action_type.dialogue->setEnabled(false);
         items.new_action_type.package->setEnabled(false);
         items.new_action_type.timer->setEnabled(false);
      }

      auto* action = menu.exec(event->globalPos());
      if (!action)
         return;

      if (action == items.new_actor) {
         this->addActor();
         return;
      }
      if (action == items.new_action_type.dialogue) {
         if (clicked_actor && clicked_phase.pointer) { // silence spurious IntelliSense warning
            auto* action = new DialogueAction;
            this->insertAction(*action, *clicked_actor, *clicked_phase.pointer);
         }
         return;
      }
      if (action == items.new_action_type.package) {
         if (clicked_actor && clicked_phase.pointer) { // silence spurious IntelliSense warning
            auto* action = new PackageAction;
            this->insertAction(*action, *clicked_actor, *clicked_phase.pointer);
         }
         return;
      }
      if (action == items.new_action_type.timer) {
         if (clicked_actor && clicked_phase.pointer) { // silence spurious IntelliSense warning
            auto* action = new TimerAction;
            this->insertAction(*action, *clicked_actor, *clicked_phase.pointer);
         }
         return;
      }
      if (action == items.new_phase_where.before_here) {
         this->insertPhaseAt(clicked_phase.index);
         return;
      }
      if (action == items.new_phase_where.after_here) {
         this->insertPhaseAt(clicked_phase.index + 1);
         return;
      }
      if (action == items.new_phase_where.at_end) {
         this->insertPhaseAt(this->_data.phases.size());
         return;
      }
      if (action == items.edit) {
         if (clicked_action) {
            this->editAction(*clicked_action);
         } else if (clicked_phase.on_header) {
            if (clicked_phase.pointer) { // silence spurious IntelliSense warning
               this->editPhase(*clicked_phase.pointer);
            }
         }
         return;
      }
      if (action == items.remove) {
         if (clicked_action) {
            this->removeAction(*clicked_action);
         } else if (clicked_phase.on_header) {
            if (clicked_phase.pointer) { // silence spurious IntelliSense warning
               this->removePhase(*clicked_phase.pointer);
            }
         }
         return;
      }
      //
      // TODO: Execute the selected action.
      //
   }
   /*virtual*/ void SceneFormVisualEditor::mouseDoubleClickEvent(QMouseEvent* event) /*override*/ {
      struct {
         Action* action = nullptr;
         Actor*  actor  = nullptr;
         Phase*  phase  = nullptr;
      } targets;

      auto pos = event->localPos().toPoint();
      for (auto* item : this->_data.actions) {
         if (item->geometry.rect.contains(pos)) {
            targets.action = item;
            break;
         }
      }
      /*for (auto* item : this->_data.actors) {
         if (item->geometry.rect.contains(pos)) {
            targets.actor = item;
            break;
         }
      }*/
      for (auto* item : this->_data.phases) {
         auto rect = item->geometry.rel.header;
         rect.moveTo(item->geometry.rect.topLeft());
         if (rect.contains(pos)) {
            targets.phase = item;
            break;
         }
      }

      if (targets.action) {
         this->editAction(*targets.action);
         return;
      }
      if (targets.phase) {
         this->editPhase(*targets.phase);
         return;
      }
   }
   /*virtual*/ void SceneFormVisualEditor::mousePressEvent(QMouseEvent* event) /*override*/ {
      if (event->button() != Qt::LeftButton && event->button() != Qt::RightButton)
         return;
      event->setAccepted(true);

      struct {
         Action* action = nullptr;
         Actor*  actor  = nullptr;
         Phase*  phase  = nullptr;
      } targets;

      auto pos = event->localPos().toPoint();
      for (auto* item : this->_data.actions) {
         if (item->geometry.rect.contains(pos)) {
            targets.action = item;
            break;
         }
      }
      /*for (auto* item : this->_data.actors) {
         if (item->geometry.rect.contains(pos)) {
            targets.actor = item;
            break;
         }
      }*/
      for (auto* item : this->_data.phases) {
         auto rect = item->geometry.rel.header;
         rect.moveTo(item->geometry.rect.topLeft());
         if (rect.contains(pos)) {
            targets.phase = item;
            break;
         }
      }

      if (targets.action) {
         this->_select(targets.action);
         return;
      }
      if (targets.phase) {
         this->_select(targets.phase);
         return;
      }
      this->_deselect_all();
   }
   /*virtual*/ void SceneFormVisualEditor::paintEvent(QPaintEvent* event) /*override*/ {
      const auto& ds = this->_data;

      int graph_bottom = 0;
      if (ds.actors.empty()) {
         for (auto* phase : ds.phases) {
            graph_bottom = std::max(graph_bottom, phase->geometry.rect.bottom());
         }
      } else {
         const auto* last = ds.actors.back();
         graph_bottom = last->geometry.rect.bottom() + this->_style.actor.padding;
      }

      QPainter painter(this);
      //
      // Draw shadows.
      //
      {
         painter.save();
         painter.translate(this->_style.shadow.thickness, this->_style.shadow.thickness);

         painter.setBrush(this->_style.shadow.color);
         painter.setPen(Qt::NoPen);
         if (!ds.phases.empty()) {
            auto* phase_s = ds.phases.front();
            auto* phase_e = ds.phases.back();

            int height = graph_bottom - phase_e->geometry.rect.y();

            QRect rect = phase_s->geometry.rect;
            rect.setRight(phase_e->geometry.rect.right());
            rect.setHeight(height);
            painter.drawRect(rect);
         }
         for (auto* actor : ds.actors) {
            painter.drawRect(actor->geometry.rect);
         }

         painter.restore();
      }
      
      StyleOption option;
      option.active = this->hasFocus();
      //
      // Draw phase grid.
      //
      {
         auto* sel = this->_selected_phase();
         for (size_t i = 0; i < ds.phases.size(); ++i) {
            auto* phase  = ds.phases[i];
            int   height = graph_bottom - phase->geometry.rect.y();

            option.selected = phase == sel;
            phase->paint(painter, this->_style, option, i + 1, height);
         }
      }
      //
      // Draw actors.
      //
      option.selected = false;
      for (auto* actor : ds.actors) {
         actor->paint(painter, this->_style);
      }
      //
      // Draw actions.
      //
      {
         auto* sel = this->_selected_action();
         for (auto* action : ds.actions) {
            option.selected = action == sel;
            action->paint(painter, this->_style, option);
         }
      }
   }
#pragma endregion

void SceneFormVisualEditor::_deselect_all() {
   if (std::holds_alternative<std::monostate>(this->_selection))
      return;
   this->_selection = {};
   this->repaint();
}
void SceneFormVisualEditor::_select(Action* item) {
   if (this->_selected_action() == item)
      return;
   this->_selection = item;
   this->repaint();
}
void SceneFormVisualEditor::_select(Phase* item) {
   if (this->_selected_phase() == item)
      return;
   this->_selection = item;
   this->repaint();
}

ui::types::conditions::context SceneFormVisualEditor::_make_condition_context() const {
   ui::types::conditions::context ctx;
   ctx.owner = this->_context.scene;
   ctx.quest = this->_context.quest;
   ctx.prefer_working_copy = true;
   return ctx;
}
void SceneFormVisualEditor::_update_phase_conditions(bool trigger_geometry_update) {
   if (this->_data.phases.empty())
      return;

   std::optional<ui::types::conditions::context> context;

   for (auto* item : this->_data.phases) {
      if (item->data.conditions.start.empty() && item->data.conditions.completion.empty())
         continue;
      if (!context.has_value()) {
         context = this->_make_condition_context();
      }
      item->recacheConditionStrings(context.value());
   }
   if (trigger_geometry_update && context.has_value()) {
      this->_update_geometry();
      this->update();
   }
}

void SceneFormVisualEditor::_update_cached_internal_relationships() {
   for (auto* actor : this->_data.actors) {
      auto& dst = actor->cached.actions;
      dst.clear();

      if (actor->alias_id == -1)
         continue;

      for (auto* action : this->_data.actions)
         if (action->base_data.alias_id == actor->alias_id)
            dst.push_back(action);
   }
}
void SceneFormVisualEditor::_update_geometry() {
   auto& ds = this->_data;

   auto font = this->fontMetrics();

   int graph_top_y      = this->_style.view_padding;
   int graph_top_height = 0;
   //
   // Compute the phase widths and the height of the graph header (phase headers and conditions).
   //
   for (size_t i = 0; i < ds.phases.size(); ++i) {
      auto* phase = ds.phases[i];
      phase->recalcSize(phase->editor_width, this->_style, font);

      int x = 0;
      if (i == 0) {
         x = this->_style.view_padding + this->_style.actor_outset;
      } else {
         x = ds.phases[i - 1]->geometry.rect.right();
      }
      phase->geometry.rect.moveTo(x, graph_top_y);

      graph_top_height = std::max(phase->geometry.rect.height(), graph_top_height);
   }
   for (auto* phase : ds.phases) {
      //
      // Each header expands to match the largest header among them.
      //
      phase->geometry.rect.setHeight(graph_top_height);
   }
   //
   // Compute the actions' X-coordinates, widths, and heights.
   //
   for (auto* action : ds.actions) {
      const auto& pi = action->base_data.phase_indices;
      if (pi.start >= ds.phases.size())
         continue;
      if (pi.end >= ds.phases.size())
         continue;
      const auto* phase_s = ds.phases[pi.start];
      const auto* phase_e = ds.phases[pi.end];

      int x_l = phase_s->geometry.rect.left()  + this->_style.action.inset;
      int x_r = phase_e->geometry.rect.right() - this->_style.action.inset;

      action->recalcSize(x_r - x_l, this->_style, font);
      action->geometry.rect.moveLeft(x_l);
   }
   //
   // Compute the actor's metrics.
   //
   for (size_t i = 0; i < ds.actors.size(); ++i) {
      auto* actor = ds.actors[i];
      auto& rect  = actor->geometry.rect;
      //
      // Compute actor's X and Y coordinates and width:
      //
      rect.setX(this->_style.view_padding);
      if (!ds.phases.empty()) {
         rect.setRight(ds.phases.back()->geometry.rect.right() + this->_style.actor_outset);
      } else {
         rect.setWidth(200);
      }
      {
         int l = this->_style.actor_outset;
         int r = rect.width() - this->_style.actor_outset;

         auto& bnd_act = actor->geometry.rel.actions;
         bnd_act.setLeft(l);
         bnd_act.setRight(r);
      }
      if (i == 0) {
         rect.setY(graph_top_y + graph_top_height);
      } else {
         auto& prev = ds.actors[i - 1]->geometry.rect;
         rect.setY(prev.bottom() + this->_style.actor.padding);
      }
      actor->doVerticalLayout(this->_style, font);
   }
   //
   // Overall size:
   //
   int graph_bottom = 0;
   if (ds.actors.empty()) {
      for (auto* phase : ds.phases) {
         graph_bottom = std::max(graph_bottom, phase->geometry.rect.bottom());
      }
   } else {
      const auto* last = ds.actors.back();
      graph_bottom = last->geometry.rect.bottom() + this->_style.actor.padding;
   }
   graph_bottom += this->_style.view_padding;

   if (ds.actors.empty()) {
      if (!ds.phases.empty()) {
         this->_cached.size.setWidth(ds.phases.back()->geometry.rect.width() + this->_style.view_padding + this->_style.actor_outset);
      } else {
         this->_cached.size.setWidth(0);
      }
   } else {
      auto& a_geo = ds.actors[0]->geometry;
      this->_cached.size.setWidth(a_geo.rect.right() + this->_style.view_padding);
   }
   this->_cached.size.setHeight(graph_bottom);
}