#pragma once
#include <QWidget>
#include "dk3d/BoundInput.h"
#include "dk3d/enums/InputDevice.h"
#include "dk3d/tools/_base.h"

namespace DK3DToolOptions {
   class Base : public QWidget {
      Q_OBJECT;
      public:
         Base(DK3D::tool_id id, QWidget* parent = nullptr) : QWidget(parent), state{ .id = id } {}

         inline DK3D::ControlType controlType() const noexcept { return this->state.controlType; }

      public slots:
         void setControlType(DK3D::ControlType);
         void showOptions(const DK3D::tools::opaque_option_union&);
         void writeTo(DK3D::tools::opaque_option_union&);

      signals:
         void controlTypeChanged(DK3D::ControlType);
         void edited();

      protected:
         struct {
            const DK3D::tool_id id = DK3D::tools::id_of_none;
            DK3D::ControlType controlType = DK3D::ControlType::Boolean;
         } state;

         virtual void _readOptions(const DK3D::tools::opaque_option_union&) = 0;
         virtual void _writeOptions(DK3D::tools::opaque_option_union&) = 0;
   };
}