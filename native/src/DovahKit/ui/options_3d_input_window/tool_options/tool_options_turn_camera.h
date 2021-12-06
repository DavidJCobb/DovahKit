#pragma once
#include "tool_options_base.h"
#include "ui_tool_options_turn_camera.h"
#include "../../../dk3d/tools/turn_camera.h"

namespace DK3DToolOptions {
   class TurnCamera : public Base {
      Q_OBJECT;
      public:
         using options_type = DK3D::tools::turn_camera::options;
      public:
         TurnCamera(QWidget* parent = nullptr);

      protected slots:
      void _onControlTypeChanged();

      private:
         Ui::ToolOptionsTurnCamera ui;

         virtual void _readOptions(const DK3D::tools::opaque_option_union&) override;
         virtual void _writeOptions(DK3D::tools::opaque_option_union&) override;
   };
}