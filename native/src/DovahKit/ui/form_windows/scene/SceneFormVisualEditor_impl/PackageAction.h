#pragma once
#include <vector>
#include "./Action.h"

namespace dovah {
   class form_stub;
}

namespace SceneFormVisualEditor_impl {
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
}