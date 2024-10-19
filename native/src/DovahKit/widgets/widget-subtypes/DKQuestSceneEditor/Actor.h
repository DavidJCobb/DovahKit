#pragma once
#include <cstdint>
#include <vector>
#include <QFontMetrics>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include "./ActorBehaviorFlag.h"

namespace DKQuestSceneEditor_impl {
   class  Action;
   struct Style;
}

namespace DKQuestSceneEditor_impl {
   class Actor {
      public:

         // Compute the Y-offsets of the actor's contents, and the actor's height. 
         // Assumes that the actor has already had its position set to where it 
         // needs to go, and updates the referenced actions' Y-offsets accordingly.
         void doVerticalLayout(const Style&, const QFontMetrics&);

         void paint(QPainter&, const Style&);

      public:
         uint32_t alias_id = -1;
         struct {
            ActorBehaviorFlag death = ActorBehaviorFlag::None;
            ActorBehaviorFlag combat = ActorBehaviorFlag::None;
            ActorBehaviorFlag dialogue = ActorBehaviorFlag::None;
            ActorBehaviorFlag observe_corpse = ActorBehaviorFlag::None;
         } behavior_flags;
         struct {
            bool no_player_activation = false;
            bool optional = false;
         } participation_flags;
         struct {
            std::vector<Action*> actions; // updated by DKQuestSceneEditor::_update_cached_internal_relationships
            QString alias_name;
         } cached;
         struct {
            QRect rect;
            struct {
               QRect actions;
            } rel;
            struct {
               int dialogue = 0;
               int package  = 0;
               int timer    = 0;
            } action_type_y_offsets;
            struct {
               int dialogue = 0;
               int package  = 0;
               int timer    = 0;
            } action_type_heights;
         } geometry;
   };
}