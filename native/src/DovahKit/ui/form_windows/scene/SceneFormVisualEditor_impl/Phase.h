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
#include "dovah/forms/components/papyrus/fragment_data/scene_fragment_data.h"
#include "dovah/forms/Scene.h"
#include "ui/types/conditions/condition.h"
#include "./ScriptFragment.h"

namespace SceneFormVisualEditor_impl {
   struct Style;
   struct StyleOption;
}

namespace SceneFormVisualEditor_impl {
   struct PhaseData {
      QString name;
      struct {
         std::vector<ui::types::conditions::condition> start;
         std::vector<ui::types::conditions::condition> completion;
      } conditions;
      struct {
         ScriptFragment start;
         ScriptFragment completion;
      } fragments;

      void importMainData(const dovah::loaded_forms::Scene::phase&);
      void importFragments(const dovah::loaded_forms::components::papyrus::scene_fragment_data::phase_fragment&);
   };

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
         PhaseData data;
         uint32_t  editor_width = 200;

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