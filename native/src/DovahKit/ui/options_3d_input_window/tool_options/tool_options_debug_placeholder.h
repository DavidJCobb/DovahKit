#pragma once
#include "tool_options_base.h"
#include <QLineEdit>
#include "../../../dk3d/tools/debug_placeholder.h"

namespace DK3DToolOptions {
   class DebugPlaceholder : public Base {
      Q_OBJECT;
      public:
         using options_type = DK3D::tools::debug_placeholder::options;
      public:
         DebugPlaceholder(QWidget* parent = nullptr);

      private:
         struct {
            QLineEdit* textbox = nullptr;
         } ui;

         virtual void _readOptions(const DK3D::tools::opaque_option_union&) override;
         virtual void _writeOptions(DK3D::tools::opaque_option_union&) override;
   };
}