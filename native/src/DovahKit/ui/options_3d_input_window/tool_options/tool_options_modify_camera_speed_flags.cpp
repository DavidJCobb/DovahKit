#include "tool_options_modify_camera_speed_flags.h"
#include <QGridLayout>
#include <QLabel>
#include "editor/subsystems/worldinput/enums/bool_operation.h"
#include "editor/subsystems/worldinput/tools/_options.h"

namespace worldinput {
   using namespace dovahkit::subsystems::worldinput;
}

namespace {
   QComboBox* _make_bool_op_select(QWidget* parent) {
      auto* widget = new QComboBox(parent);
      widget->addItem("Do nothing", (int)worldinput::bool_operation::no_op);
      widget->addItem("Turn on",    (int)worldinput::bool_operation::set_true);
      widget->addItem("Turn off",   (int)worldinput::bool_operation::set_false);
      widget->addItem("Invert",     (int)worldinput::bool_operation::invert);
      return widget;
   }

   void _set_bool_op_select(QComboBox& widget, worldinput::bool_operation v) {
      widget.setCurrentIndex(widget.findData((int)v));
   }
}

namespace DK3DToolOptions {
   ModifyCameraSpeedFlags::ModifyCameraSpeedFlags(QWidget* parent) : Base(worldinput::id_of_tool<tool_type>(), parent) {
      /*QObject::connect(this, &Base::controlTypeChanged, this, [this]() {
      });*/
      //
      auto* layout = new QGridLayout(this);
      this->ui.boost     = _make_bool_op_select(this);
      this->ui.precision = _make_bool_op_select(this);
      layout->addWidget(new QLabel(tr("Boost:"), this), 0, 0);
      layout->addWidget(this->ui.boost, 0, 1);
      layout->addWidget(new QLabel(tr("Precision:"), this), 1, 0);
      layout->addWidget(this->ui.precision, 1, 1);
      layout->setContentsMargins({ 0, 0, 0, 0 });
      //
      QObject::connect(this->ui.boost,     QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Base::edited); // signal-to-signal
      QObject::connect(this->ui.precision, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Base::edited); // signal-to-signal
   }

   void ModifyCameraSpeedFlags::_readOptions(const opaque_option_union& oou) {
      const auto* o = worldinput::tools::option_union::as<tool_type>(oou);
      if (!o)
         return;
      const auto blocker0 = QSignalBlocker(this->ui.boost);
      const auto blocker1 = QSignalBlocker(this->ui.precision);
      _set_bool_op_select(*this->ui.boost,     o->boost);
      _set_bool_op_select(*this->ui.precision, o->precision);
   }
   void ModifyCameraSpeedFlags::_writeOptions(opaque_option_union& oou) {
      auto* o = worldinput::tools::option_union::as<tool_type>(oou);
      if (!o)
         return;
      o->boost     = (worldinput::bool_operation)this->ui.boost->currentData().toInt();
      o->precision = (worldinput::bool_operation)this->ui.precision->currentData().toInt();
   }
}