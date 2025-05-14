#include "./Actor.h"
#include "./Style.h"
#include "../SceneFormVisualEditor.h" // for QObject::tr

#include "./DialogueAction.h"
#include "./PackageAction.h"
#include "./TimerAction.h"

namespace SceneFormVisualEditor_impl {
   void Actor::doVerticalLayout(const Style& style, const QFontMetrics& font) {
      //
      // Compute the inner height. This entails computing the max height for each 
      // action type.
      //
      auto& geo = this->geometry;

      auto& bnd_act = geo.rel.actions;
      bnd_act.setY(font.lineSpacing() + style.actor.padding * 2);

      auto& max_action_heights = geo.action_type_heights;
      auto& action_y_offsets   = geo.action_type_y_offsets;
      max_action_heights = {}; // reset
      for (auto* action : this->cached.actions) {
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
         *dst = std::max(action->geometry.rect.height(), *dst);
      }
      //
      // We'll want to compute the Y-coordinates for each action type, too, with 
      // padding only being present between types that are present.
      //
      action_y_offsets.dialogue = 0;
      action_y_offsets.timer  = action_y_offsets.dialogue;
      if (int h = max_action_heights.dialogue) {
         action_y_offsets.timer += h + style.action.inset;
      }
      action_y_offsets.package = action_y_offsets.timer;
      if (int h = max_action_heights.timer) {
         action_y_offsets.package += h + style.action.inset;
      }
      //
      // We now have all the information we need to compute the actor's height.
      //
      bnd_act.setHeight(action_y_offsets.package + max_action_heights.package + style.action.inset);
      geo.rect.setHeight(bnd_act.bottom());
      //
      // Set the Y-coordinates for this actor's actions.
      //
      int action_base_y = geo.rect.y() + bnd_act.y();
      for (auto* action : this->cached.actions) {
         auto& rect = action->geometry.rect;
         if (dynamic_cast<DialogueAction*>(action)) {
            rect.moveTop(action_base_y + action_y_offsets.dialogue);
         } else if (dynamic_cast<PackageAction*>(action)) {
            rect.moveTop(action_base_y + action_y_offsets.package);
         } else if (dynamic_cast<TimerAction*>(action)) {
            rect.moveTop(action_base_y + action_y_offsets.timer);
         }
      }


   }
   void Actor::paint(QPainter& painter, const Style& style) {
      painter.setBrush(QBrush(style.actor.background));
      {
         auto pen = QPen(style.actor.text);
         pen.setCosmetic(true);
         painter.setPen(pen);
      }

      painter.drawRect(this->geometry.rect);

      painter.save();
      painter.translate(this->geometry.rect.topLeft());
         
      QString text = SceneFormVisualEditor::tr("%1: D(%2), C(%3), PD(%4), OC(%5)");
      text = text.arg(this->cached.alias_name);
         
      auto _flag_str = [](ActorBehaviorFlags& flags) {
         QString out;
         if (flags.pause) {
            out = SceneFormVisualEditor::tr("P", "actor behavior flag");
            if (flags.end) {
               out += ", ";
               out += SceneFormVisualEditor::tr("E", "actor behavior flag");
            }
         } else if (flags.end) {
            out = SceneFormVisualEditor::tr("E", "actor behavior flag");
         } else {
            out = SceneFormVisualEditor::tr("N", "actor behavior flag");
         }
         return out;
      };
      text = text.arg(_flag_str(this->behavior_flags.death));
      text = text.arg(_flag_str(this->behavior_flags.combat));
      text = text.arg(_flag_str(this->behavior_flags.dialogue));
      text = text.arg(_flag_str(this->behavior_flags.observe_corpse));

      int  padding = style.actor.padding;
      auto rect    = this->geometry.rect;
      rect.moveTo(0, 0);
      rect.adjust(padding, padding, -padding, -padding);
      painter.drawText(rect, text);

      painter.restore();
   }
}