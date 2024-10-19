#include "./DKQuestSceneEditor.h"
#include <QPaintEvent>
#include <QPainter>
#include "dovah/form_stub.h"
#include "./widget-subtypes/DKQuestSceneEditor/Actor.h"
#include "./widget-subtypes/DKQuestSceneEditor/Phase.h"

#include "./widget-subtypes/DKQuestSceneEditor/DialogueAction.h"
#include "./widget-subtypes/DKQuestSceneEditor/PackageAction.h"
#include "./widget-subtypes/DKQuestSceneEditor/TimerAction.h"

namespace {
   using namespace DKQuestSceneEditor_impl;
}

DKQuestSceneEditor::DKQuestSceneEditor(QWidget* parent) : QWidget(parent) {
}

DKQuestSceneEditor::~DKQuestSceneEditor() {
   this->clear();
}

void DKQuestSceneEditor::clear() {
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
void DKQuestSceneEditor::spawnRenderTest() {
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

#pragma region Overrides
   /*virtual*/ QSize DKQuestSceneEditor::minimumSizeHint() const /*override*/ {
      return this->sizeHint();
   }
   /*virtual*/ QSize DKQuestSceneEditor::sizeHint() const /*override*/ {
      return this->_cached.size;
   }

   /*virtual*/ void DKQuestSceneEditor::paintEvent(QPaintEvent* event) /*override*/ {
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

void DKQuestSceneEditor::_update_cached_internal_relationships() {
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
void DKQuestSceneEditor::_update_geometry() {
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