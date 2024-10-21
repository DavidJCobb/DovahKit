#include "./SceneFormVisualEditor.h"
#include <QPaintEvent>
#include <QPainter>
#include "dovah/form_stubs/helpers/get_unique_outbound_use.h"
#include "dovah/form_stub.h"
#include "dovah/forms/Quest.h"
#include "dovah/forms/Scene.h"
#include "./SceneFormVisualEditor_impl/Actor.h"
#include "./SceneFormVisualEditor_impl/Phase.h"

#include "./SceneFormVisualEditor_impl/DialogueAction.h"
#include "./SceneFormVisualEditor_impl/PackageAction.h"
#include "./SceneFormVisualEditor_impl/TimerAction.h"

#include "ui/form_windows/quest/QuestAllDialogueDatastore.h"

#include <QBoxLayout>
#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include <QTableView>
#include "./SceneActorBehaviorModel.h"
#include "./SceneActorParticipationModel.h"

namespace {
   constexpr const bool only_import_actions_belonging_to_defined_actors = true;
}

namespace {
   using namespace SceneFormVisualEditor_impl;

   using loaded_form_type = dovah::loaded_forms::Scene;

   constexpr const uint32_t no_alias = (uint32_t)-1;
}

SceneFormVisualEditor::SceneFormVisualEditor(QWidget* parent) : QWidget(parent) {
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
      if (item->alias_id == id) {
         #if _DEBUG
            if constexpr (only_import_actions_belonging_to_defined_actors) {
               __debugbreak(); // This action shouldn't be here!
            }
         #endif
         return true;
      }
      if (const auto* casted = dynamic_cast<const DialogueAction*>(item))
         if (casted->headtrack_alias_id == id)
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
   this->clear();

   if (!this->_context.scene)
      return;
   const auto* working = this->_context.scene->get_working_copy();
   if (!working)
      return;
   const auto& form = *(loaded_form_type*)working;

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

         dst->name = QString::fromStdString(src.name);
         {
            auto&  src_list = src.conditions.start;
            auto&  dst_list = dst->conditions.start;
            size_t size     = src_list.size();
            dst_list.resize(size);
            for (size_t i = 0; i < size; ++i) {
               dst_list[i] = ui::types::conditions::condition(src_list[i]);
            }
         }
         {
            auto&  src_list = src.conditions.completion;
            auto&  dst_list = dst->conditions.completion;
            size_t size     = src_list.size();
            dst_list.resize(size);
            for (size_t i = 0; i < size; ++i) {
               dst_list[i] = ui::types::conditions::condition(src_list[i]);
            }
         }
         dst->editor_width = src.editor_display_width;
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

            casted_dst->topic = casted->topic.get_form_stub();
            casted_dst->headtrack_alias_id = casted->headtrack_alias_id;
            casted_dst->emotion.type  = casted->emotion.type;
            casted_dst->emotion.value = casted->emotion.value;
            casted_dst->looping.min = casted->looping.min;
            casted_dst->looping.max = casted->looping.max;
         } else if (auto* casted = std::get_if<src_type::package_data>(&src.data)) {
            auto* casted_dst = new PackageAction;
            dst = casted_dst;

            casted_dst->packages.reserve(casted->packages.size());
            for (auto& use : casted->packages)
               if (use)
                  casted_dst->packages.push_back(use.get_form_stub());
         } else if (auto* casted = std::get_if<src_type::timer_data>(&src.data)) {
            auto* casted_dst = new TimerAction;
            dst = casted_dst;

            casted_dst->duration = casted->duration;
         }
         this->_data.actions.push_back(dst);
         dst->name      = QString::fromStdString(src.name);
         dst->alias_id  = src.alias_id;
         dst->action_id = src.action_id;
         {
            dst->flags.face_target      = src.flags & src_type::flag::face_target;
            dst->flags.looping          = src.flags & src_type::flag::looping;
            dst->flags.headtrack_player = src.flags & src_type::flag::headtrack_player;
         }
         dst->phase_indices.start = src.phase_indices.start;
         dst->phase_indices.end   = src.phase_indices.end;
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
            if (!casted->topic)
               continue;
            auto* data = this->_context.dialogue->item_for_topic_stub(*casted->topic);
            if (!data)
               continue;

            for (auto* info : data->infos) {
               dst.push_back(info->cached.responses);
            }
         }
      }
   }
   this->_update_geometry();
}
void SceneFormVisualEditor::exportData() {
   if (!this->_context.scene)
      return;
   auto* working = this->_context.scene->get_working_copy();
   if (!working)
      return;
   auto& form = *(loaded_form_type*)working;

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
         dst_item.name = src_item.name.toStdString();
         _copy_conditions(src_item.conditions.start, dst_item.conditions.start);
         _copy_conditions(src_item.conditions.completion, dst_item.conditions.completion);
         dst_item.editor_display_width = src_item.editor_width;
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
         dst_item.name = src_item.name.toStdString();
         dst_item.alias_id = src_item.alias_id;
         dst_item.action_id = src_item.action_id;
         {
            dst_item.flags &= ~(
               dst_type::flag::face_target |
               dst_type::flag::headtrack_player |
               dst_type::flag::looping
               );
            if (src_item.flags.face_target)
               dst_item.flags |= dst_type::flag::face_target;
            if (src_item.flags.headtrack_player)
               dst_item.flags |= dst_type::flag::headtrack_player;
            if (src_item.flags.looping)
               dst_item.flags |= dst_type::flag::looping;
         }
         dst_item.phase_indices.start = src_item.phase_indices.start;
         dst_item.phase_indices.end = src_item.phase_indices.end;
         if (auto* casted = dynamic_cast<DialogueAction*>(&src_item)) {
            auto& dst_data = dst_item.data.emplace<dst_type::dialogue_data>();
            dst_data.topic.set(form, casted->topic);
            dst_data.emotion.type  = casted->emotion.type;
            dst_data.emotion.value = casted->emotion.value;
            dst_data.looping.min = casted->looping.min;
            dst_data.looping.max = casted->looping.max;
            dst_data.headtrack_alias_id = casted->headtrack_alias_id;
         } else if (auto* casted = dynamic_cast<PackageAction*>(&src_item)) {
            auto& dst_data = dst_item.data.emplace<dst_type::package_data>();
            {
               auto& dst_list = dst_data.packages;
               for (auto* item : casted->packages) {
                  if (!item)
                     continue;
                  dst_list.emplace_back().set(form, item);
               }
            }
         } else if (auto* casted = dynamic_cast<TimerAction*>(&src_item)) {
            auto& dst_data = dst_item.data.emplace<dst_type::timer_data>();
            dst_data.duration = casted->duration;
         }
      }
   }
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
      for (auto* actor : this->_data.actors) {
         model->commitActor(*actor);
      }
   }
   dialog->deleteLater();
}

void SceneFormVisualEditor::clear() {
   this->_context = {};
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
   this->_data.phases[7]->name = "Ready for Erikur distraction start (s)";
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
      actor->behavior_flags.dialogue = ActorBehaviorFlag::Pause;
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
      action->action_id = 15;
      action->alias_id  = pausing_aliases[0].second;
      action->phase_indices = { 2, 2 };
   }
   {
      auto* action = new DialogueAction;
      this->_data.actions.push_back(action);
      action->action_id = 5;
      action->alias_id  = pausing_aliases[0].second;
      action->phase_indices = { 3, 3 };

      action->cached.infos.push_back("There's a likely-looking filly. Even if she is an elf.");
   }
   {
      auto* action = new DialogueAction;
      this->_data.actions.push_back(action);
      action->action_id = 6;
      action->alias_id  = pausing_aliases[0].second;
      action->phase_indices = { 4, 4 };

      action->cached.infos.push_back("You there! Serving girl! What's your name, dear?");
   }
   {
      auto* action = new DialogueAction;
      this->_data.actions.push_back(action);
      action->action_id = 8;
      action->alias_id  = pausing_aliases[0].second;
      action->phase_indices = { 6, 6 };

      action->cached.infos.push_back("No, no, that's not what I'm interested in right now. | I just wanted to get a better look at you. I like what I see, my dear. | And believe me, I don't say that to everyone. I'm very discriminating when it comes to the female form.");
   }
   {
      auto* action = new DialogueAction;
      this->_data.actions.push_back(action);
      action->action_id = 12;
      action->alias_id  = pausing_aliases[0].second;
      action->phase_indices = { 8, 8 };

      action->cached.infos.push_back("Oh... not at the moment. Maybe later. Don't go far.");
   }
   {
      auto* action = new PackageAction;
      this->_data.actions.push_back(action);
      action->action_id = 30;
      action->alias_id  = pausing_aliases[0].second;
      action->phase_indices = { 0, 3 };

      action->packages.push_back(nullptr);
      action->cached.package_editor_ids.push_back("MQ201ErikurApproachBrelas");
   }
   {
      auto* action = new PackageAction;
      this->_data.actions.push_back(action);
      action->action_id = 2;
      action->alias_id  = pausing_aliases[0].second;
      action->phase_indices = { 4, 10 };

      action->packages.push_back(nullptr);
      action->cached.package_editor_ids.push_back("DefaultStayAtCurrentLocationScene");
   }
   {
      auto* action = new TimerAction;
      this->_data.actions.push_back(action);
      action->action_id = 16;
      action->alias_id  = pausing_aliases[0].second;
      action->phase_indices = { 2, 2 };

      action->duration = 1.0F;
   }
   #pragma endregion

   this->_update_cached_internal_relationships();
   this->_update_geometry();
   this->updateGeometry();
}

void SceneFormVisualEditor::removeActor(uint32_t id) {
   bool needs_geo_update = false;

   needs_geo_update = std::erase_if(this->_data.actors, [id](Actor* a) { return a->alias_id == id; }) != 0;

   auto&  list = this->_data.actions;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* action = list[i];
      if (action->alias_id == id) {
         delete action;
         list.erase(list.begin() + i);
         --i;
         --size;
         needs_geo_update = true;
         continue;
      }
      if (auto* casted = dynamic_cast<DialogueAction*>(action)) {
         if (casted->headtrack_alias_id == id)
            casted->headtrack_alias_id = no_alias;
      }
   }

   if (needs_geo_update) {
      this->_update_geometry();
      this->update();
   }
}

#pragma region Overrides
   /*virtual*/ QSize SceneFormVisualEditor::minimumSizeHint() const /*override*/ {
      return this->sizeHint();
   }
   /*virtual*/ QSize SceneFormVisualEditor::sizeHint() const /*override*/ {
      return this->_cached.size;
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
      //
      // Draw phase grid.
      //
      for (size_t i = 0; i < ds.phases.size(); ++i) {
         auto* phase  = ds.phases[i];
         int   height = graph_bottom - phase->geometry.rect.y();
         phase->paint(painter, this->_style, i + 1, height);
      }
      //
      // Draw actors.
      //
      for (auto* actor : ds.actors) {
         actor->paint(painter, this->_style);
      }
      //
      // Draw actions.
      //
      for (auto* action : ds.actions) {
         action->paint(painter, this->_style);
      }
   }
#pragma endregion

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
      if (item->conditions.start.empty() && item->conditions.completion.empty())
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
         if (action->alias_id == actor->alias_id)
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
      if (action->phase_indices.start >= ds.phases.size())
         continue;
      if (action->phase_indices.end >= ds.phases.size())
         continue;
      const auto* phase_s = ds.phases[action->phase_indices.start];
      const auto* phase_e = ds.phases[action->phase_indices.end];

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