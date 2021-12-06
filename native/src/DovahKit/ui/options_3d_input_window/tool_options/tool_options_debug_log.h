#pragma once
#include "tool_options_base.h"
#include <QSpinBox>
#include "../../../dk3d/tools/debug_log.h"

namespace DK3DToolOptions {
   class DebugLog : public Base {
      Q_OBJECT;
      public:
         using options_type = DK3D::tools::debug_log::options;
      public:
         DebugLog(QWidget* parent = nullptr);

      private:
         struct {
            QSpinBox* spinbox = nullptr;
         } ui;

         virtual void _readOptions(const DK3D::tools::opaque_option_union&) override;
         virtual void _writeOptions(DK3D::tools::opaque_option_union&) override;
   };
}