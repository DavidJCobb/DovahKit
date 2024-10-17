#include "./DKQuestSceneEditor.h"
#include <QPaintEvent>
#include <QPainter>
#include "ui/form_windows/scene/SceneDatastore.h"

DKQuestSceneEditor::DKQuestSceneEditor(QWidget* parent = nullptr);
      
#pragma region Overrides
   /*virtual*/ void DKQuestSceneEditor::paintEvent(QPaintEvent* event) /*override*/ {
      if (!this->datastore)
         return;
      const auto& ds = *this->datastore;

      QPainter painter(this);
      {
         QPen border(QColor(0, 0, 0));
         border.setCosmetic(true);
         painter.setPen(border);
      }
      //
      // Draw shadows.
      //
      {
         const auto shadow_offset = QPoint(this->_display_metrics.shadow_size, this->_display_metrics.shadow_size);
         painter.setBrush(QColor(170, 170, 170));
         if (!ds.phases.empty()) {
            auto* phase_s = ds.phases.front();
            auto* phase_e = ds.phases.back();

            QRect bounds;
            bounds.setTopLeft(phase_s->display_geometry.overall.topLeft());
            bounds.setBottomRight(phase_e->display_geometry.overall.bottomRight());
            bounds.moveTo(bounds.topLeft() + shadow_offset);

            painter.drawRect(bounds);
         }
         for (auto* actor : ds.actors) {
            auto bounds = actor->display_geometry.overall;
            bounds.moveTo(bounds.topLeft() + shadow_offset);

            painter.drawRect(bounds);
         }
      }
      //
      // Draw phase grid.
      //
      painter.setBrush(QColor(205, 205, 205));
      for (size_t i = 0; i < ds.phases.size(); ++i) {
         auto* phase = ds.phases[i];

         painter.drawRect(phase->display_geometry.overall);

         painter.drawRect(phase->display_geometry.header);
         QString header_text;
         if (phase->name.isEmpty()) {
            header_text = tr("Phase %1: %2").arg(i + 1).arg(phase->name);
         } else {
            header_text = tr("Phase %1").arg(i + 1);
         }
         painter.drawText(phase->display_geometry.header, Qt::AlignTop | Qt::AlignHCenter, header_text);

         if (auto& rect = phase->display_geometry.conditions.start; !rect.isNull()) {
            auto text = tr("Start conditions:\n%1").arg(phase->cached.conditions.start);
            painter.drawRect(rect);
            painter.drawText(rect, Qt::AlignTop | Qt::AlignLeft, text);
         }
         if (auto& rect = phase->display_geometry.conditions.completion; !rect.isNull()) {
            auto text = tr("Completion conditions:\n%1").arg(phase->cached.conditions.completion);
            painter.drawRect(rect);
            painter.drawText(rect, Qt::AlignTop | Qt::AlignLeft, text);
         }
      }
      //
      // Draw actors.
      //
      painter.setBrush(QColor(255, 255, 255));
      for (auto* actor : ds.actors) {
         painter.drawRect(actor->display_geometry.overall);

         QString text = tr("%1: D(%2), C(%3), PD(%4), OC(%5)");
         text = text.arg(actor->cached.alias_name);
         
         auto _flag_str = [](SceneDatastore::ActorBehaviorFlag flag) {
            using enum SceneDatastore::ActorBehaviorFlag;
            switch (flag) {
               case Pause:
                  return tr("P", "actor behavior flag");
               case End:
                  return tr("E", "actor behavior flag");
            }
            return tr("N", "actor behavior flag");
         };
         text = text.arg(_flag_str(actor->behavior_flags.death));
         text = text.arg(_flag_str(actor->behavior_flags.combat));
         text = text.arg(_flag_str(actor->behavior_flags.dialogue));
         text = text.arg(_flag_str(actor->behavior_flags.observe_corpse));

         QRect pos = actor->display_geometry.overall;
         pos.setTopLeft(pos.topLeft() + QPoint(this->_display_metrics.actor_padding, this->_display_metrics.actor_padding));
         painter.drawText(pos, text);
      }
      //
      // Draw actions.
      //
      auto _draw_typical_header = [&painter](SceneDatastore::Action& action, QColor color) {
         QString header_text;
         if (action.name.isEmpty()) {
            header_text = tr("Action %1: %2").arg(action.action_id).arg(action.name);
         } else {
            header_text = tr("Action %1").arg(action.action_id);
         }
         painter.setBrush(QColor(205, 205, 205));
         painter.drawRect(action.display_geometry.header);
         painter.drawText(action.display_geometry.header, Qt::AlignTop | Qt::AlignHCenter, header_text);
      };
      for (auto* action : ds.actions) {
         if (auto* casted = dynamic_cast<SceneDatastore::DialogueAction*>(action)) {
            _draw_typical_header(*action, QColor(205, 205, 205));

            painter.setBrush(QColor(255, 255, 255));
            static_assert(false, "TODO: Draw the infos");
         } else if (auto* casted = dynamic_cast<SceneDatastore::PackageAction*>(action)) {
            _draw_typical_header(*action, QColor(172, 208, 213));

            painter.setBrush(QColor(255, 255, 255));
            static_assert(false, "TODO: Draw the packages");
         } else if (auto* casted = dynamic_cast<SceneDatastore::TimerAction*>(action)) {
            QString header_text;
            if (casted->name.isEmpty()) {
               header_text = tr("Action %1: %2 (%3 seconds)").arg(casted->action_id).arg(casted->name);
            } else {
               header_text = tr("Action %1 (%2 seconds)").arg(casted->action_id);
            }
            header_text = header_text.arg(casted->duration);

            painter.setBrush(QColor(255, 255, 0));
            painter.drawRect(action->display_geometry.header);
            painter.drawText(action->display_geometry.header, Qt::AlignTop | Qt::AlignHCenter, header_text);
         }
      }
   }
#pragma endregion

SceneDatastore* DKQuestSceneEditor::datastore() const;
void DKQuestSceneEditor::setDatastore(SceneDatastore*);

void DKQuestSceneEditor::_update_geometry() {
   if (!this->datastore)
      return;
   auto& ds = *this->datastore;

   for (auto* item : ds.actions)
      item->display_geometry = {};
   for (auto* item : ds.actors)
      item->display_geometry = {};
   for (auto* item : ds.phases)
      item->display_geometry = {};

   auto font = this->fontMetrics();

   int graph_top_height = 0;
   //
   // Compute the phase widths and the height of the graph header (phase headers and conditions).
   //
   for (size_t i = 0; i < ds.phases.size(); ++i) {
      auto* phase  = ds.phases[i];
      auto& bounds = phase->display_geometry.overall;
      if (i == 0) {
         bounds.setX(this->_display_metrics.view_padding + this->_display_metrics.action_inset);
      } else {
         bounds.setX(ds.phases[i - 1]->display_geometry.overall.right());
      }
      bounds.setWidth(phase->editor_width);
      bounds.setY(this->_display_metrics.view_padding);

      auto& bnd_header = phase->display_geometry.header;
      auto  margin     = this->_display_metrics.phase_header_margin;
      bnd_header.setX(bounds.x() + margin);
      bnd_header.setY(bounds.y() + margin);
      bnd_header.setRight(bounds.right() - margin);
      bnd_header.setHeight(this->_display_metrics.phase_header_line_count * font.lineSpacing());

      auto& bnd_cnd_s = phase->display_geometry.conditions.start;
      bnd_cnd_s.setX(bounds.x() + margin);
      bnd_cnd_s.setRight(bounds.right() - margin);
      if (!phase->cached.conditions.start.isEmpty()) {
         bnd_cnd_s.setY(bnd_cnd_s.bottom() + margin * 2);
         bnd_cnd_s.setHeight(this->_display_metrics.phase_conditions_line_count * font.lineSpacing());
      } else {
         bnd_cnd_s.setY(bnd_cnd_s.bottom());
      }

      auto& bnd_cnd_c = phase->display_geometry.conditions.completion;
      bnd_cnd_c.setX(bounds.x() + margin);
      bnd_cnd_c.setRight(bounds.right() - margin);
      if (!phase->cached.conditions.completion.isEmpty()) {
         bnd_cnd_c.setY(bnd_cnd_s.bottom() + margin * 2);
         bnd_cnd_c.setHeight(this->_display_metrics.phase_conditions_line_count * font.lineSpacing());
      } else {
         bnd_cnd_c.setY(bnd_cnd_s.bottom());
      }

      graph_top_height = std::max(graph_top_height, bnd_cnd_c.bottom() + margin);
      bounds.setHeight(graph_top_height);
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

      auto& bounds = action->display_geometry.overall;
      bounds.setX(phase_s->display_geometry.overall.x() + this->_display_metrics.action_inset);
      bounds.setRight(phase_e->display_geometry.overall.right() - this->_display_metrics.action_inset);

      int height = 0;
      if (auto* casted = dynamic_cast<SceneDatastore::DialogueAction*>(action)) {
         static_assert(false, "TODO: Compute the action's height.");
      } else if (auto* casted = dynamic_cast<SceneDatastore::PackageAction*>(action)) {
         static_assert(false, "TODO: Compute the action's height.");
      } else if (auto* casted = dynamic_cast<SceneDatastore::TimerAction*>(action)) {
         height = font.lineSpacing() * 3;
      }
      bounds.setHeight(height);
   }
   //
   // Compute the actor's metrics.
   //
   for (size_t i = 0; i < ds.actors.size(); ++i) {
      auto* actor  = ds.actors[i];
      auto& bounds = actor->display_geometry.overall;
      //
      // Compute actor's X and Y coordinates and width:
      //
      bounds.setX(this->_display_metrics.view_padding);
      if (!ds.phases.empty()) {
         bounds.setRight(ds.phases.back()->display_geometry.overall.right() + this->_display_metrics.view_padding);
      } else {
         bounds.setWidth(200);
      }
      if (i == 0) {
         bounds.setY(graph_top_height);
      } else {
         auto& bnd_prev = ds.actors[i - 1]->display_geometry.overall;
         bounds.setY(bnd_prev.bottom() + this->_display_metrics.actor_padding);
      }
      //
      // Compute the inner height. This entails computing the max height for each 
      // action type.
      //
      auto& bnd_act = actor->display_geometry.actions;
      bnd_act = bounds;
      bnd_act.setY(bounds.y() + font.lineSpacing());

      auto& max_action_heights = actor->display_geometry.action_type_heights;
      auto& action_y_offsets   = actor->display_geometry.action_type_y_offsets;
      for (auto* action : actor->cached.actions) {
         int height = action->display_geometry.overall.height();
         if (auto* casted = dynamic_cast<SceneDatastore::DialogueAction*>(action)) {
            if (height > max_action_heights.dialogue)
               max_action_heights.dialogue = height;
         } else if (auto* casted = dynamic_cast<SceneDatastore::PackageAction*>(action)) {
            if (height > max_action_heights.package)
               max_action_heights.package = height;
         } else if (auto* casted = dynamic_cast<SceneDatastore::TimerAction*>(action)) {
            if (height > max_action_heights.timer)
               max_action_heights.timer = height;
         }
      }
      //
      // We'll want to compute the Y-coordinates for each action type, too, with 
      // padding only being present between types that are present.
      //
      action_y_offsets.dialogue = 0;
      action_y_offsets.package  = action_y_offsets.dialogue;
      if (int h = max_action_heights.dialogue) {
         action_y_offsets.package += h + this->_display_metrics.action_inset;
      }
      action_y_offsets.timer = max_action_heights.package;
      if (int h = max_action_heights.package) {
         action_y_offsets.timer += h + this->_display_metrics.action_inset;
      }
      //
      // We now have all the information we need to compute the actor's height.
      //
      bnd_act.setHeight(action_y_offsets.timer + max_action_heights.timer);
      bounds.setBottom(bnd_act.bottom());
      //
      // Set the Y-coordinates for this actor's actions.
      //
      for (auto* action : actor->cached.actions) {
         auto& bounds = action->display_geometry.overall;
         if (auto* casted = dynamic_cast<SceneDatastore::DialogueAction*>(action)) {
            bounds.moveTop(action_y_offsets.dialogue);
         } else if (auto* casted = dynamic_cast<SceneDatastore::PackageAction*>(action)) {
            bounds.moveTop(action_y_offsets.package);
         } else if (auto* casted = dynamic_cast<SceneDatastore::TimerAction*>(action)) {
            bounds.moveTop(action_y_offsets.timer);
         }
      }
   }
   //
   // Set the phase heights.
   //
   if (!ds.actors.empty()) {
      auto* last   = ds.actors.back();
      int   bottom = last->display_geometry.overall.bottom() + this->_display_metrics.actor_padding;
      for (auto* phase : ds.phases) {
         phase->display_geometry.overall.setBottom(bottom);
      }
   }
}