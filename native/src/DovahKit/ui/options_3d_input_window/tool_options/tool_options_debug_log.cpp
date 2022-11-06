#include "tool_options_debug_log.h"
#include <QGridLayout>
#include <QLabel>
#include "editor/subsystems/worldinput/tools/_options.h"

namespace worldinput {
   using namespace dovahkit::subsystems::worldinput;
}

namespace DK3DToolOptions {
   DebugLog::DebugLog(QWidget* parent) : Base(worldinput::id_of_tool<tool_type>(), parent) {
      /*QObject::connect(this, &Base::controlTypeChanged, this, [this]() {
      });*/
      //
      auto* layout = new QGridLayout(this);
      this->ui.spinbox = new QSpinBox(this);
      layout->addWidget(new QLabel(tr("Number:"), this), 0, 0);
      layout->addWidget(this->ui.spinbox, 0, 1);
      layout->setContentsMargins({ 0, 0, 0, 0 });
      //
      this->ui.spinbox->setRange(0, 255);
      QObject::connect(this->ui.spinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Base::edited); // signal-to-signal
   }

   void DebugLog::_readOptions(const opaque_option_union& oou) {
      const auto* o = worldinput::tools::option_union::as<tool_type>(oou);
      if (!o)
         return;
      const auto blocker = QSignalBlocker(this->ui.spinbox);
      this->ui.spinbox->setValue(o->number);
   }
   void DebugLog::_writeOptions(opaque_option_union& oou) {
      auto* o = worldinput::tools::option_union::as<tool_type>(oou);
      if (!o)
         return;
      o->number = this->ui.spinbox->value();
   }
}