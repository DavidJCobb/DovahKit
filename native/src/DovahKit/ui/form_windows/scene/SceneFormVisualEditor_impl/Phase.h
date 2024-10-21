#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <QFontMetrics>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include "ui/types/conditions/condition.h"

namespace SceneFormVisualEditor_impl {
   struct Style;
   struct StyleOption;
}

namespace SceneFormVisualEditor_impl {
   class Phase {
      public:
         struct Fragment {
            std::string scriptname;
            std::string function;
         };

      public:
         void paint(QPainter&, const Style&, const StyleOption&, int index, int height);
         void recacheConditionStrings(const ui::types::conditions::context&);
         void recalcSize(int width, const Style&, const QFontMetrics&);

      public:
         QString name;
         struct {
            std::vector<ui::types::conditions::condition> start;
            std::vector<ui::types::conditions::condition> completion;
         } conditions;
         struct {
            Fragment start;
            Fragment completion;
         } fragments;
         uint32_t editor_width = 200;

         struct {
            struct {
               QString start;
               QString completion;
            } conditions;
         } cached;
         struct {
            QRect rect;
            struct {
               QRect header;
               struct {
                  QRect start;
                  QRect completion;
               } conditions;
            } rel;
         } geometry;
   };
}