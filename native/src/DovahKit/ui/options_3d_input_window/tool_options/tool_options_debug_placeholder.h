#pragma once
#include "tool_options_base.h"
#include <QLineEdit>
#include "editor/subsystems/worldinput/tools/debug_placeholder.h"

namespace DK3DToolOptions {
   class DebugPlaceholder : public Base {
      Q_OBJECT;
      public:
         using tool_type = dovahkit::subsystems::worldinput::tools::debug_placeholder;
      public:
         DebugPlaceholder(QWidget* parent = nullptr);

      private:
         struct {
            QLineEdit* textbox = nullptr;
         } ui;

         virtual void _readOptions(const opaque_option_union&) override;
         virtual void _writeOptions(opaque_option_union&) override;
   };
}