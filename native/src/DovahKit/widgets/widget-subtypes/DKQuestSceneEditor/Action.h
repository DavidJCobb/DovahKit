#pragma once
#include <cstdint>
#include <QFontMetrics>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>

namespace DKQuestSceneEditor_impl {
   struct Style;
}

namespace DKQuestSceneEditor_impl {
   class Action {
      public:
         virtual ~Action() = default;

         virtual void paint(QPainter&, const Style&) = 0;
         virtual void recalcSize(int width, const Style&, const QFontMetrics&) = 0;

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
            QRect rect;
            struct {
               QRect header;
               QRect body;
            } rel;
         } geometry;
   };
}