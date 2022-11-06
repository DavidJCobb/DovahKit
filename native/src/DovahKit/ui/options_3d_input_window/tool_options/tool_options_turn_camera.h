#pragma once
#include "tool_options_base.h"
#include "ui_tool_options_turn_camera.h"
#include "editor/subsystems/worldinput/tools/turn_camera.h"

namespace DK3DToolOptions {
   class TurnCamera : public Base {
      Q_OBJECT;
      public:
         using tool_type = dovahkit::subsystems::worldinput::tools::turn_camera;
      public:
         TurnCamera(QWidget* parent = nullptr);

      protected slots:
      void _onControlTypeChanged();

      private:
         Ui::ToolOptionsTurnCamera ui;

         virtual void _readOptions(const opaque_option_union&) override;
         virtual void _writeOptions(opaque_option_union&) override;
   };
}