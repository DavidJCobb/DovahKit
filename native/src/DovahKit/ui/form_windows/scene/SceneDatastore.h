#pragma once
#include <cstdint>
#include <vector>
#include <QRect>
#include <QString>
#include "dovah/data/dialogue/emotion.h"
#include "ui/types/conditions/condition.h"

namespace dovah {
   class form_stub;
}

class SceneDatastore {
   public:
      enum class ActorBehaviorFlag {
         None,
         Pause,
         End,
      };

      class Action;

      class Actor {
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
               std::vector<Action*> actions;
               QString alias_name;
            } cached;
            struct {
               QRect overall;
               QRect actions;
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
            } display_geometry;
      };

      class Phase {
         public:
            QString name;
            struct {
               std::vector<ui::types::conditions::condition> start;
               std::vector<ui::types::conditions::condition> completion;
            } conditions;
            uint32_t editor_width = 200;

            struct {
               struct {
                  QString start;
                  QString completion;
               } conditions;
            } cached;
            struct {
               QRect overall; // note: height is undefined
               QRect header;
               struct {
                  QRect start;
                  QRect completion;
               } conditions;
            } display_geometry;
      };

      class Action {
         public:
            virtual ~Action() = default;

         public:
            QString  name;
            uint32_t alias_id  = -1;
            uint32_t action_id = 0;
            struct {
               bool face_target      = false;
               bool looping          = false;
               bool headtrack_player = false;
            } flags;
            struct {
               uint32_t start = 0;
               uint32_t end   = 0;
            } phase_indices;
            struct {
               QRect overall;
               QRect header;
               QRect body;
            } display_geometry;
      };
      class DialogueAction : public Action {
         public:
            dovah::form_stub* topic = nullptr;
            uint32_t headtrack_alias_id = -1;
            struct {
               dovah::dialogue::emotion type = dovah::dialogue::emotion::neutral;
               uint32_t value = 50;
            } emotion;
            struct {
               float min = 0.0F;
               float max = 0.0F;
            } looping;

            struct {
               std::vector<QString> infos;
            } cached;
      };
      class PackageAction : public Action {
         public:
            std::vector<dovah::form_stub*> packaces;
            struct {
               std::vector<QString> package_editor_ids;
            } cached;
      };
      class TimerAction : public Action {
         public:
            float duration = 0.0F; // seconds
      };

   public:
      std::vector<Actor*>  actors;
      std::vector<Phase*>  phases;
      std::vector<Action*> actions;
};
