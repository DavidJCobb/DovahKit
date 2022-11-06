#pragma once
#include <QWidget>
#include "editor/subsystems/worldinput/enums/control_type.h"
#include "editor/subsystems/worldinput/tools/_base.h"

namespace DK3DToolOptions {
   class Base : public QWidget {
      Q_OBJECT;
      public:
         using tool_id = dovahkit::subsystems::worldinput::tool_id;
         using control_type = dovahkit::subsystems::worldinput::control_type;
         using opaque_option_union = dovahkit::subsystems::worldinput::tools::opaque_option_union;

      public:
         Base(tool_id id, QWidget* parent = nullptr) : QWidget(parent), state{ .id = id } {}

         inline control_type controlType() const noexcept { return this->state.controlType; }

      public slots:
         void setControlType(control_type);
         void showOptions(const opaque_option_union&);
         void writeTo(opaque_option_union&);

      signals:
         void controlTypeChanged(control_type);
         void edited();

      protected:
         struct {
            const tool_id id = dovahkit::subsystems::worldinput::tools::id_of_none;
            control_type controlType = control_type::button;
         } state;

         virtual void _readOptions(const opaque_option_union&) = 0;
         virtual void _writeOptions(opaque_option_union&) = 0;
   };
}