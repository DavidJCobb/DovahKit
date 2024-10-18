#include "./DKQuestSceneEditor.h"
#include <QPaintEvent>
#include <QPainter>
#include "dovah/form_stub.h"
#include "editor/helpers/form_identifiers_to_string.h"

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

#pragma region Entity types
   #pragma region Phase
      void DKQuestSceneEditor::Phase::paint(QPainter& painter, const Style& style, int index, int height) {
         painter.setBrush(QBrush(style.phase.background));
         painter.setPen(QPen(style.phase.text));

         painter.save();
         painter.translate(this->geometry.position);

         QRect rect;
         rect.setWidth(this->geometry.size.width());
         rect.setHeight(height);
         painter.drawRect(rect);

         {  // Header
            QString text;
            if (this->name.isEmpty()) {
               text = tr("Action %1: %2").arg(index).arg(this->name);
            } else {
               text = tr("Action %1").arg(index);
            }
            const QRect& rect = this->geometry.rel.header;
            painter.drawRect(rect);
            painter.drawText(rect, Qt::AlignTop | Qt::AlignHCenter, text);
         }
         if (auto src = this->cached.conditions.start; !src.isEmpty()) {
            QString text = tr("Start conditions:\n") + src;

            const QRect& rect = this->geometry.rel.conditions.start;
            painter.drawRect(rect);
            painter.drawText(rect, Qt::AlignTop | Qt::AlignLeft, text);
         }
         if (auto src = this->cached.conditions.completion; !src.isEmpty()) {
            QString text = tr("Completion conditions:\n") + src;

            const QRect& rect = this->geometry.rel.conditions.completion;
            painter.drawRect(rect);
            painter.drawText(rect, Qt::AlignTop | Qt::AlignLeft, text);
         }

         painter.restore();
      }
      void DKQuestSceneEditor::Phase::recalcSize(int width, const Style& style, const QFontMetrics& font_metrics) {
         this->geometry.size.setWidth(width);

         int height    = 0;
         int box_width = width - style.phase.header_box.margin * 2;
         {
            auto& box = this->geometry.rel.header;
            box.setX(style.phase.header_box.margin);
            box.setY(style.phase.header_box.margin);
            box.setWidth(box_width);
            box.setHeight(font_metrics.lineSpacing() * 2);
            height = box.bottom() + style.phase.header_box.margin;
         }
         int condition_box_height = font_metrics.lineSpacing() * 4;
         if (this->cached.conditions.start.isEmpty()) {
            this->geometry.rel.conditions.start = {};
         } else {
            int y = this->geometry.rel.header.bottom() + style.phase.header_box.margin * 2;

            auto& box  = this->geometry.rel.conditions.start;
            auto& prev = this->geometry.rel.header;
            box.setX(style.phase.header_box.margin);
            box.setY(y);
            box.setWidth(box_width);
            box.setHeight(condition_box_height);
            height = box.bottom() + style.phase.header_box.margin;
         }
         if (this->cached.conditions.completion.isEmpty()) {
            this->geometry.rel.conditions.completion = {};
         } else {
            int y = style.phase.header_box.margin * 2;
            if (this->cached.conditions.start.isEmpty()) {
               y += this->geometry.rel.header.bottom();
            } else {
               y += this->geometry.rel.conditions.start.bottom();
            }

            auto& box  = this->geometry.rel.conditions.start;
            auto& prev = this->geometry.rel.header;
            box.setX(style.phase.header_box.margin);
            box.setY(y);
            box.setWidth(box_width);
            box.setHeight(condition_box_height);
            height = box.bottom() + style.phase.header_box.margin;
         }

         this->geometry.size.setHeight(height);
      }
   #pragma endregion
      
   #pragma region Phase
      void DKQuestSceneEditor::Actor::paint(QPainter& painter, const Style& style) {
         painter.setBrush(QBrush(style.actor.background));
         painter.setPen(QPen(style.actor.text));

         painter.save();
         painter.translate(this->geometry.position);

         QRect rect({ 0, 0 }, this->geometry.size);
         painter.drawRect(rect);
         
         QString text = tr("%1: D(%2), C(%3), PD(%4), OC(%5)");
         text = text.arg(this->cached.alias_name);
         
         auto _flag_str = [](ActorBehaviorFlag flag) {
            using enum ActorBehaviorFlag;
            switch (flag) {
               case ActorBehaviorFlag::Pause:
                  return tr("P", "actor behavior flag");
               case ActorBehaviorFlag::End:
                  return tr("E", "actor behavior flag");
               case ActorBehaviorFlag::None:
                  return tr("N", "actor behavior flag");
            }
            return tr("?", "actor behavior flag");
         };
         text = text.arg(_flag_str(this->behavior_flags.death));
         text = text.arg(_flag_str(this->behavior_flags.combat));
         text = text.arg(_flag_str(this->behavior_flags.dialogue));
         text = text.arg(_flag_str(this->behavior_flags.observe_corpse));

         int padding = style.actor.padding;
         rect.adjust(padding, padding, -padding, -padding);
         painter.drawText(rect, text);

         painter.restore();
      }
   #pragma endregion

   #pragma region Action
      void DKQuestSceneEditor::Action::drawCell(QPainter& painter, const Style& style, QRect rect, QString text, int align_flags) const {
         QPen border_pen;
         border_pen.setCosmetic(true);
         border_pen.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
         border_pen.setWidth(1);
         //
         // Draw border outer:
         //
         border_pen.setColor(style.action.cell.border.outer_dark);
         painter.setPen(border_pen);
         painter.drawLine(rect.topLeft(), rect.topRight());
         painter.drawLine(rect.topLeft(), rect.bottomLeft());
         border_pen.setColor(style.action.cell.border.outer_light);
         painter.setPen(border_pen);
         painter.drawLine(rect.bottomRight(), rect.bottomLeft());
         painter.drawLine(rect.bottomRight(), rect.topRight());
         rect.adjust(1, 1, -1, -1);
         //
         // Draw border inner:
         //
         border_pen.setColor(style.action.cell.border.inner_dark);
         painter.setPen(border_pen);
         painter.drawLine(rect.topLeft(), rect.topRight());
         painter.drawLine(rect.topLeft(), rect.bottomLeft());
         border_pen.setColor(style.action.cell.border.inner_light);
         painter.setPen(border_pen);
         painter.drawLine(rect.bottomRight(), rect.bottomLeft());
         painter.drawLine(rect.bottomRight(), rect.topRight());
         rect.adjust(1, 1, -1, -1);
         //
         painter.setBrush(QBrush(style.action.cell.background));
         painter.setPen(Qt::NoPen);
         painter.drawRect(rect);
         //
         // Draw text:
         //
         int padding = style.action.cell.padding;
         rect.adjust(padding, padding, -padding, -padding); // row padding
         painter.setPen(QPen(style.action.cell.text));
         painter.drawText(rect, align_flags, text);
      }
   #pragma endregion
   #pragma region DialogueAction
      /*virtual*/ void DKQuestSceneEditor::DialogueAction::paint(QPainter& painter, const Style& style) /*override*/ {
         painter.save();
         painter.translate(this->geometry.position);

         QString text;
         if (!this->name.isEmpty()) {
            text = tr("Action %1: %2").arg(this->action_id).arg(this->name);
         } else {
            text = tr("Action %1").arg(this->action_id);
         }

         painter.setBrush(QBrush(style.action.dialogue_header.background));
         painter.setPen(QPen(style.action.dialogue_header.text));
         painter.drawRect(QRect{ QPoint{ 0, 0 }, this->geometry.size });
         painter.drawRect(this->geometry.rel.header);
         painter.drawText(this->geometry.rel.header, Qt::AlignTop | Qt::AlignHCenter, text);

         int cell_flags = Qt::AlignTop | Qt::AlignLeft;
         if (style.show_all_text)
            cell_flags |= Qt::TextWordWrap;

         size_t size = this->cached.infos.size();
         int    y    = this->geometry.rel.body.y();
         for (size_t i = 0; i < size; ++i) {
            auto text   = this->cached.infos[i];
            auto height = this->body_geometry.infos[i];

            QRect row = this->geometry.rel.body;
            row.setY(y);
            row.setHeight(height);
            this->drawCell(painter, style, row, text, cell_flags);

            y += height;
         }

         painter.restore();
      }
      /*virtual*/ void DKQuestSceneEditor::DialogueAction::recalcSize(int width, const Style& style, const QFontMetrics& font_metrics) /*override*/ {
         this->geometry.size.setWidth(width);

         int height = font_metrics.lineSpacing(); // header
         height += style.action.header_padding * 2;
         height += 1; // border
         const int header_height = height;
         this->geometry.rel.header = QRect(QPoint{ 0, 0 }, QSize{ width, header_height });

         size_t count = this->cached.infos.size();
         auto&  dst   = this->body_geometry.infos;
         dst.clear();
         dst.resize(count);
         for (size_t i = 0; i < count; ++i) {
            if (style.show_all_text) {
               auto text_rect = font_metrics.boundingRect(
                  0,
                  0,
                  width,
                  std::numeric_limits<int>::max(),
                  Qt::TextSingleLine | Qt::TextWordWrap,
                  this->cached.infos[i]
               );
               dst[i] = text_rect.bottom();
            } else {
               dst[i] = font_metrics.lineSpacing();
            }
            dst[i] += style.action.cell.padding * 2;
            dst[i] += cell_border_width * 2;

            height += dst[i];
         }

         this->geometry.rel.body = QRect(QPoint{ 0, header_height }, QSize{ width, height - header_height });
         this->geometry.size.setHeight(height);
      }
   #pragma endregion
   #pragma region PackageAction
      /*virtual*/ void DKQuestSceneEditor::PackageAction::paint(QPainter& painter, const Style& style) /*override*/ {
         painter.save();
         painter.translate(this->geometry.position);

         QString text;
         if (!this->name.isEmpty()) {
            text = tr("Action %1: %2").arg(this->action_id).arg(this->name);
         } else {
            text = tr("Action %1").arg(this->action_id);
         }

         painter.setBrush(QBrush(style.action.package_header.background));
         painter.setPen(QPen(style.action.package_header.text));
         painter.drawRect(QRect{ QPoint{ 0, 0 }, this->geometry.size });
         painter.drawRect(this->geometry.rel.header);
         painter.drawText(this->geometry.rel.header, Qt::AlignTop | Qt::AlignHCenter, text);

         size_t size = this->packages.size();
         for (size_t i = 0; i < size; ++i) {
            int y = this->geometry.rel.body.y() + this->body_geometry.row_height * i;

            QRect row = this->geometry.rel.body;
            row.setY(y);
            row.setHeight(this->body_geometry.row_height);

            QString form_id   = "00000000";
            QString editor_id;
            if (i < this->cached.package_editor_ids.size()) {
               editor_id = this->cached.package_editor_ids[i];
            }
            if (auto* stub = this->packages[i]) {
               form_id = editor_helpers::form_id_to_string(stub->formID);
               if (editor_id.isEmpty())
                  editor_id = QString::fromStdString(stub->editorID);
            }

            {
               QRect cell = row;
               cell.setWidth(this->body_geometry.form_id_width);
               this->drawCell(painter, style, cell, form_id, Qt::AlignTop | Qt::AlignLeft);
            }
            if (row.width() > this->body_geometry.form_id_width) {
               QRect cell = row;
               cell.setX(cell.x() + this->body_geometry.form_id_width);
               this->drawCell(painter, style, cell, editor_id, Qt::AlignTop | Qt::AlignHCenter);
            }
         }

         painter.restore();
      }
      /*virtual*/ void DKQuestSceneEditor::PackageAction::recalcSize(int width, const Style& style, const QFontMetrics& font_metrics) /*override*/ {
         this->geometry.size.setWidth(width);

         int height = font_metrics.lineSpacing(); // header
         height += style.action.header_padding * 2;
         height += 1; // border
         const int header_height = height;
         this->geometry.rel.header = QRect(QPoint{ 0, 0 }, QSize{ width, header_height });

         this->body_geometry.row_height = font_metrics.lineSpacing();
         this->body_geometry.row_height += style.action.cell.padding * 2;
         this->body_geometry.row_height += cell_border_width * 2;
         height += this->body_geometry.row_height * this->packages.size();

         this->geometry.rel.body = QRect(QPoint{ 0, header_height }, QSize{ width, height - header_height });

         this->body_geometry.form_id_width = font_metrics.width("00000000");
         this->body_geometry.form_id_width += style.action.cell.padding * 2;
         this->body_geometry.form_id_width += cell_border_width * 2;

         this->geometry.size.setHeight(height);
      }
   #pragma endregion
   #pragma region TimerAction
      /*virtual*/ void DKQuestSceneEditor::TimerAction::paint(QPainter& painter, const Style& style) /*override*/ {
         painter.save();
         painter.translate(this->geometry.position);

         QString text;
         if (!this->name.isEmpty()) {
            text = tr("Action %1: %2 (%3 seconds)").arg(this->action_id).arg(this->name);
         } else {
            text = tr("Action %1 (%2 seconds)").arg(this->action_id);
         }
         text = text.arg(this->duration);

         QRect rect({ 0, 0 }, this->geometry.size);
         painter.setBrush(QBrush(style.action.timer.background));
         painter.setPen(QPen(style.action.timer.text));
         painter.drawRect(rect);
         rect.adjust(0, style.action.header_padding, 0, 0);
         painter.drawText(rect, Qt::AlignTop | Qt::AlignHCenter, text);

         painter.restore();
      }
      /*virtual*/ void DKQuestSceneEditor::TimerAction::recalcSize(int width, const Style& style, const QFontMetrics& font_metrics) /*override*/ {
         this->geometry.size.setWidth(width);
         this->geometry.size.setHeight(font_metrics.lineSpacing() * 3 + style.action.header_padding * 2);
      }
   #pragma endregion
#pragma endregion
      
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
            graph_bottom = std::max(graph_bottom, phase->geometry.bottom());
         }
      } else {
         const auto* last = ds.actors.back();
         graph_bottom = last->geometry.bottom() + this->_style.actor.padding;
      }

      QPainter painter(this);
      //
      // Draw shadows.
      //
      {
         const auto shadow_offset = QPoint(this->_style.shadow.thickness, this->_style.shadow.thickness);
         painter.setBrush(this->_style.shadow.color);
         painter.setPen(Qt::NoPen);
         if (!ds.phases.empty()) {
            auto* phase_s = ds.phases.front();
            auto* phase_e = ds.phases.back();

            int height = graph_bottom - phase_e->geometry.position.y();

            QRect rect;
            rect.setTopLeft(phase_s->geometry.position + shadow_offset);
            rect.setRight(phase_e->geometry.right());
            rect.setHeight(height);
            rect.moveTopLeft(rect.topLeft() + shadow_offset);
            painter.drawRect(rect);
         }
         for (auto* actor : ds.actors) {
            QRect rect(actor->geometry.position, actor->geometry.size);
            rect.moveTopLeft(rect.topLeft() + shadow_offset);

            painter.drawRect(rect);
         }
      }
      //
      // Draw phase grid.
      //
      for (size_t i = 0; i < ds.phases.size(); ++i) {
         auto* phase = ds.phases[i];
         phase->paint(painter, this->_style, i + 1, graph_bottom - phase->geometry.position.y());
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

      auto& pos = phase->geometry.position;
      pos.setY(graph_top_y);
      if (i == 0) {
         pos.setX(this->_style.view_padding + this->_style.actor_outset);
      } else {
         auto& prev = ds.phases[i - 1]->geometry;
         pos.setX(prev.position.x() + prev.size.width());
      }

      graph_top_height = std::max(phase->geometry.size.height(), graph_top_height);
   }
   for (auto* phase : ds.phases) {
      phase->geometry.size.setHeight(graph_top_height);
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

      int x_l = phase_s->geometry.left()  + this->_style.action.inset;
      int x_r = phase_e->geometry.right() - this->_style.action.inset;

      action->recalcSize(x_r - x_l, this->_style, font);
      action->geometry.position.setX(x_l);
   }
   //
   // Compute the actor's metrics.
   //
   for (size_t i = 0; i < ds.actors.size(); ++i) {
      auto* actor    = ds.actors[i];
      auto& geo      = actor->geometry;
      auto& position = actor->geometry.position;
      //
      // Compute actor's X and Y coordinates and width:
      //
      geo.setX(this->_style.view_padding);
      if (!ds.phases.empty()) {
         geo.setRight(ds.phases.back()->geometry.right() + this->_style.actor_outset);
      } else {
         geo.size.setWidth(200);
      }
      if (i == 0) {
         geo.setY(graph_top_y + graph_top_height);
      } else {
         auto& geo_prev = ds.actors[i - 1]->geometry;
         geo.setY(geo_prev.bottom() + this->_style.actor.padding);
      }
      //
      // Compute the inner height. This entails computing the max height for each 
      // action type.
      //
      auto& bnd_act = actor->geometry.rel.actions;
      bnd_act = {};
      bnd_act.setY(font.lineSpacing() + this->_style.actor.padding * 2);

      auto& max_action_heights = geo.action_type_heights;
      auto& action_y_offsets   = geo.action_type_y_offsets;
      for (auto* action : actor->cached.actions) {
         int* dst = nullptr;
         if (dynamic_cast<DialogueAction*>(action)) {
            dst = &max_action_heights.dialogue;
         } else if (dynamic_cast<PackageAction*>(action)) {
            dst = &max_action_heights.package;
         } else if (dynamic_cast<TimerAction*>(action)) {
            dst = &max_action_heights.timer;
         }
         if (!dst)
            continue;
         *dst = std::max(action->geometry.size.height(), *dst);
      }
      //
      // We'll want to compute the Y-coordinates for each action type, too, with 
      // padding only being present between types that are present.
      //
      action_y_offsets.dialogue = 0;
      action_y_offsets.timer  = action_y_offsets.dialogue;
      if (int h = max_action_heights.dialogue) {
         action_y_offsets.timer += h + this->_style.action.inset;
      }
      action_y_offsets.package = action_y_offsets.timer;
      if (int h = max_action_heights.timer) {
         action_y_offsets.package += h + this->_style.action.inset;
      }
      //
      // We now have all the information we need to compute the actor's height.
      //
      bnd_act.setHeight(action_y_offsets.package + max_action_heights.package + this->_style.action.inset);
      geo.size.setHeight(bnd_act.bottom());
      //
      // Set the Y-coordinates for this actor's actions.
      //
      int action_base_y = geo.position.y() + bnd_act.y();
      for (auto* action : actor->cached.actions) {
         auto& pos = action->geometry.position;
         if (dynamic_cast<DialogueAction*>(action)) {
            pos.setY(action_base_y + action_y_offsets.dialogue);
         } else if (dynamic_cast<PackageAction*>(action)) {
            pos.setY(action_base_y + action_y_offsets.package);
         } else if (dynamic_cast<TimerAction*>(action)) {
            pos.setY(action_base_y + action_y_offsets.timer);
         }
      }
   }
   //
   // Overall size:
   //
   int graph_bottom = 0;
   if (ds.actors.empty()) {
      for (auto* phase : ds.phases) {
         graph_bottom = std::max(graph_bottom, phase->geometry.bottom());
      }
   } else {
      const auto* last = ds.actors.back();
      graph_bottom = last->geometry.bottom() + this->_style.actor.padding;
   }
   graph_bottom += this->_style.view_padding;

   if (ds.actors.empty()) {
      if (!ds.phases.empty()) {
         this->_cached.size.setWidth(ds.phases.back()->geometry.size.width() + this->_style.view_padding + this->_style.actor_outset);
      } else {
         this->_cached.size.setWidth(0);
      }
   } else {
      auto& a_geo = ds.actors[0]->geometry;
      this->_cached.size.setWidth(a_geo.right() + this->_style.view_padding);
   }
   this->_cached.size.setHeight(graph_bottom);
}