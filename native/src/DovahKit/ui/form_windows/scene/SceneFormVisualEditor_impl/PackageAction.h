#pragma once
#include <vector>
#include "./Action.h"

namespace dovah {
   class form_stub;
}

namespace SceneFormVisualEditor_impl {
   struct PackageActionData {
      std::vector<dovah::form_stub*> packages;
   };

   class PackageAction : public Action {
      public:
         virtual ActionType type() const noexcept override { return ActionType::Package; };
         virtual void paint(QPainter&, const Style&, const StyleOption&) override;
         virtual void recalcSize(int width, const Style&, const QFontMetrics&) override;

      public:
         PackageActionData data;
         struct {
            std::vector<QString> package_editor_ids;
         } cached;
         struct {
            int form_id_width = 0;
            int row_height    = 0;
         } body_geometry;
   };
}