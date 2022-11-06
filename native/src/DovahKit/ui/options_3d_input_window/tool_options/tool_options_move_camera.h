#pragma once
#include "tool_options_base.h"
#include "ui_tool_options_move_camera.h"
#include "editor/subsystems/worldinput/tools/move_camera.h"

namespace DK3DToolOptions {
   class MoveCamera : public Base {
      Q_OBJECT;
      public:
         using tool_type = dovahkit::subsystems::worldinput::tools::move_camera;
      public:
         MoveCamera(QWidget* parent = nullptr);

      protected slots:
         void _onControlTypeChanged();

      private:
         Ui::ToolOptionsMoveCamera ui;

         virtual void _readOptions(const opaque_option_union&) override;
         virtual void _writeOptions(opaque_option_union&) override;
   };
}