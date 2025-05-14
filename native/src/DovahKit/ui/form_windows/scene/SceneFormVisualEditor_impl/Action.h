#pragma once
#include <cstdint>
#include <QFontMetrics>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include "./ActionType.h"

namespace SceneFormVisualEditor_impl {
   struct Style;
   struct StyleOption;
}

namespace SceneFormVisualEditor_impl {
   struct BaseActionData {
      QString  name;
      uint32_t alias_id  = -1;
      uint32_t action_id =  0;
      struct {
         uint32_t start = 0;
         uint32_t end   = 0;
      } phase_indices;
   };

   class Action {
      public:
         virtual ~Action() = default;

         virtual ActionType type() const noexcept = 0;
         virtual void paint(QPainter&, const Style&, const StyleOption&) = 0;
         virtual void recalcSize(int width, const Style&, const QFontMetrics&) = 0;
         virtual void forceOverwriteWidth(int width); // used for visual updates while actions are being resized.

      protected:
         static constexpr const int cell_border_width = 2;
         static constexpr const int cell_padding      = 3;

         void drawCell(QPainter&, const Style&, QRect, QString, int align_flags = 0) const;

      public:
         BaseActionData base_data;
         struct {
            QRect rect;
            struct {
               QRect header;
               QRect body;
            } rel;
         } geometry;
   };
}