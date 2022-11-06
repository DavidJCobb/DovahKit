#pragma once
#include "tool_options_base.h"
#include <QSpinBox>
#include "editor/subsystems/worldinput/tools/debug_log.h"

namespace DK3DToolOptions {
   class DebugLog : public Base {
      Q_OBJECT;
      public:
         using tool_type = dovahkit::subsystems::worldinput::tools::debug_log;
      public:
         DebugLog(QWidget* parent = nullptr);

      private:
         struct {
            QSpinBox* spinbox = nullptr;
         } ui;

         virtual void _readOptions(const opaque_option_union&) override;
         virtual void _writeOptions(opaque_option_union&) override;
   };
}