#pragma once
#include "tool_options_base.h"
#include <QComboBox>
#include "editor/subsystems/worldinput/tools/attempt_on_screen_selection.h"

namespace DK3DToolOptions {
   class AttemptOnScreenSelection : public Base {
      Q_OBJECT;
      public:
         using tool_type = dovahkit::subsystems::worldinput::tools::attempt_on_screen_selection;
      public:
         AttemptOnScreenSelection(QWidget* parent = nullptr);

      private:
         struct {
            QComboBox* operation = nullptr;
            QComboBox* pointer   = nullptr;
         } ui;

         virtual void _readOptions(const opaque_option_union&) override;
         virtual void _writeOptions(opaque_option_union&) override;
   };
}