#include "tool_options_debug_log.h"
#include <QGridLayout>
#include <QLabel>
#include "dk3d/tools/_options.h"

namespace DK3DToolOptions {
   DebugLog::DebugLog(QWidget* parent) : Base(DK3D::id_of_tool_options<options_type>(), parent) {
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

   void DebugLog::_readOptions(const DK3D::tools::opaque_option_union& oou) {
      const auto* o = DK3D::tools::option_union::as<options_type>(oou);
      if (!o)
         return;
      const auto blocker = QSignalBlocker(this->ui.spinbox);
      this->ui.spinbox->setValue(o->number);
   }
   void DebugLog::_writeOptions(DK3D::tools::opaque_option_union& oou) {
      auto* o = DK3D::tools::option_union::as<options_type>(oou);
      if (!o)
         return;
      o->number = this->ui.spinbox->value();
   }
}