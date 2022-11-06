#pragma once
#include "tool_options_base.h"
#include <QComboBox>
#include "editor/subsystems/worldinput/tools/modify_camera_speed_flags.h"

namespace DK3DToolOptions {
   class ModifyCameraSpeedFlags : public Base {
      Q_OBJECT;
      public:
         using tool_type = dovahkit::subsystems::worldinput::tools::modify_camera_speed_flags;
      public:
         ModifyCameraSpeedFlags(QWidget* parent = nullptr);

      private:
         struct {
            QComboBox* boost     = nullptr;
            QComboBox* precision = nullptr;
         } ui;

         virtual void _readOptions(const opaque_option_union&) override;
         virtual void _writeOptions(opaque_option_union&) override;
   };
}