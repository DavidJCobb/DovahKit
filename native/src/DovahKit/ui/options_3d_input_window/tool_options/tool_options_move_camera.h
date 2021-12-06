#pragma once
#include "tool_options_base.h"
#include "ui_tool_options_move_camera.h"
#include "../../../dk3d/tools/move_camera.h"

namespace DK3DToolOptions {
   class MoveCamera : public Base {
      Q_OBJECT;
      public:
         using options_type = DK3D::tools::move_camera::options;
      public:
         MoveCamera(QWidget* parent = nullptr);

      protected slots:
         void _onControlTypeChanged();

      private:
         Ui::ToolOptionsMoveCamera ui;

         virtual void _readOptions(const DK3D::tools::opaque_option_union&) override;
         virtual void _writeOptions(DK3D::tools::opaque_option_union&) override;
   };
}