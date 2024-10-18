#pragma once
#include <cstdint>
#include <QWidget>
#include "dovah/data/dialogue/emotion.h"
#include "ui/types/conditions/condition.h"

namespace dovah {
   class form_stub;
}

class DKQuestSceneEditor : public QWidget {
   Q_OBJECT;
   public:
      DKQuestSceneEditor(QWidget* parent = nullptr);
      ~DKQuestSceneEditor();

      enum class ActorBehaviorFlag {
         None,
         Pause,
         End,
      };

      void clear();
      void spawnRenderTest();
      
   protected:
      struct Style {
         bool show_all_text = true;

         int    view_padding = 30;
         int    actor_outset = 20;
         struct {
            QColor color     = { 170, 170, 170 };
            int    thickness = 3;
         } shadow;
         struct {
            QColor background = { 205, 205, 205 };
            QColor border     = { 0, 0, 0 };
            QColor text       = { 0, 0, 0 };
            struct {
               int margin = 5;
            } header_box;
         } phase;
         struct {
            QColor background = { 255, 255, 255 };
            QColor border     = { 0, 0, 0 };
            QColor text       = { 0, 0, 0 };
            int    padding = 10;
         } actor;
         struct { // action
            QColor border = { 0, 0, 0 };
            struct {
               QColor background = { 255, 255, 255 };
               QColor text       = { 0, 0, 0 };
               struct {
                  QColor outer_dark  = { 160, 160, 160 };
                  QColor outer_light = { 255, 255, 255 };
                  QColor inner_dark  = { 105, 105, 105 };
                  QColor inner_light = { 227, 227, 227 };
               } border;
               int padding = 3;
            } cell;
            int header_padding = 3;
            int inset = 10;

            struct {
               QColor background = { 205, 205, 205 };
               QColor text       = { 0, 0, 0 };
            } dialogue_header;
            struct {
               QColor background = { 172, 208, 213 };
               QColor text       = { 0, 0, 0 };
            } package_header;
            struct {
               QColor background = { 255, 255, 0 };
               QColor text       = { 0, 0, 0 };
            } timer;
         } action;
      };

      #pragma region Entity types
         class Action;

         class Actor {
            public:
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
                  std::vector<Action*> actions;
                  QString alias_name;
               } cached;
               struct {
                  QPoint position;
                  QSize  size;
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

                  constexpr int right() const noexcept { return this->position.x() + this->size.width(); }
                  constexpr int bottom() const noexcept { return this->position.y() + this->size.height(); }

                  constexpr void setX(int x) noexcept { this->position.setX(x); }
                  constexpr void setY(int y) noexcept { this->position.setY(y); }
                  constexpr void setRight(int x) noexcept { this->size.setWidth(x - this->position.x()); }
               } geometry;
         };

         class Phase {
            public:
               void paint(QPainter&, const Style&, int index, int height);
               void recalcSize(int width, const Style&, const QFontMetrics&);

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
                  QPoint position;
                  QSize  size;
                  struct {
                     QRect header;
                     struct {
                        QRect start;
                        QRect completion;
                     } conditions;
                  } rel;

                  constexpr int left() const noexcept { return this->position.x(); }
                  constexpr int right() const noexcept { return this->position.x() + this->size.width(); }
                  constexpr int bottom() const noexcept { return this->position.y() + this->size.height(); }
               } geometry;
         };

         class Action {
            public:
               virtual ~Action() = default;

               virtual void paint(QPainter&, const Style&) = 0;
               virtual void recalcSize(int width, const Style&, const QFontMetrics&) = 0;

               void setPosition(const QPoint& pos);

            protected:
               static constexpr const int cell_border_width = 2;
               static constexpr const int cell_padding      = 3;

               void drawCell(QPainter&, const Style&, QRect, QString, int align_flags = 0) const;

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
                  QPoint position;
                  QSize  size;
                  struct {
                     QRect header;
                     QRect body;
                  } rel;
               } geometry;
         };
         class DialogueAction : public Action {
            public:
               virtual void paint(QPainter&, const Style&) override;
               virtual void recalcSize(int width, const Style&, const QFontMetrics&) override;

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
               struct {
                  std::vector<int> infos;
               } body_geometry;
         };
         class PackageAction : public Action {
            public:
               virtual void paint(QPainter&, const Style&) override;
               virtual void recalcSize(int width, const Style&, const QFontMetrics&) override;

            public:
               std::vector<dovah::form_stub*> packages;
               struct {
                  std::vector<QString> package_editor_ids;
               } cached;
               struct {
                  int form_id_width = 0;
                  int row_height    = 0;
               } body_geometry;
         };
         class TimerAction : public Action {
            public:
               virtual void paint(QPainter&, const Style&) override;
               virtual void recalcSize(int width, const Style&, const QFontMetrics&) override;

            public:
               float duration = 0.0F; // seconds
         };
      #pragma endregion

   public:
      #pragma region Overrides
         virtual QSize minimumSizeHint() const override;
         virtual QSize sizeHint() const override;

         virtual void paintEvent(QPaintEvent* event) override;
      #pragma endregion

   protected:
      struct {
         QSize size;
      } _cached;
      struct {
         std::vector<Actor*>  actors;
         std::vector<Phase*>  phases;
         std::vector<Action*> actions;
      } _data;
      Style _style;

      void _update_cached_internal_relationships();
      void _update_geometry();
};