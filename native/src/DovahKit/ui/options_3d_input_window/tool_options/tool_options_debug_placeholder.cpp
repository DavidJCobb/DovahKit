#include "tool_options_debug_placeholder.h"
#include <QGridLayout>
#include <QLabel>
#include "editor/subsystems/worldinput/tools/_options.h"

namespace worldinput {
   using namespace dovahkit::subsystems::worldinput;
}

namespace DK3DToolOptions {
   DebugPlaceholder::DebugPlaceholder(QWidget* parent) : Base(worldinput::id_of_tool<tool_type>(), parent) {
      /*QObject::connect(this, &Base::controlTypeChanged, this, [this]() {
      });*/
      //
      auto* layout = new QGridLayout(this);
      this->ui.textbox = new QLineEdit(this);
      layout->addWidget(new QLabel(tr("Text:"), this), 0, 0);
      layout->addWidget(this->ui.textbox, 0, 1);
      layout->setContentsMargins({ 0, 0, 0, 0 });
      QObject::connect(this->ui.textbox, &QLineEdit::textEdited, this, &Base::edited); // signal-to-signal
   }

   void DebugPlaceholder::_readOptions(const opaque_option_union& oou) {
      const auto* o = worldinput::tools::option_union::as<tool_type>(oou);
      if (!o)
         return;
      const auto blocker = QSignalBlocker(this->ui.textbox);
      this->ui.textbox->setText(o->text);
   }
   void DebugPlaceholder::_writeOptions(opaque_option_union& oou) {
      auto* o = worldinput::tools::option_union::as<tool_type>(oou);
      if (!o)
         return;
      o->text = this->ui.textbox->text();
   }
}