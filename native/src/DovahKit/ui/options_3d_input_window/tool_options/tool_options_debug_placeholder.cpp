#include "tool_options_debug_placeholder.h"
#include <QGridLayout>
#include "dk3d/tools/_options.h"

namespace DK3DToolOptions {
   DebugPlaceholder::DebugPlaceholder(QWidget* parent) : Base(DK3D::id_of_tool_options<options_type>(), parent) {
      /*QObject::connect(this, &Base::controlTypeChanged, this, [this]() {
      });*/
      //
      auto* layout = new QGridLayout(this);
      this->ui.textbox = new QLineEdit(this);
      layout->addWidget(this->ui.textbox, 0, 0);
      QObject::connect(this->ui.textbox, &QLineEdit::textEdited, this, &Base::edited); // signal-to-signal
   }

   void DebugPlaceholder::_readOptions(const DK3D::tools::opaque_option_union& oou) {
      const auto* o = DK3D::tools::option_union::as<options_type>(oou);
      if (!o)
         return;
      const auto blocker = QSignalBlocker(this->ui.textbox);
      this->ui.textbox->setText(o->text);
   }
   void DebugPlaceholder::_writeOptions(DK3D::tools::opaque_option_union& oou) {
      auto* o = DK3D::tools::option_union::as<options_type>(oou);
      if (!o)
         return;
      o->text = this->ui.textbox->text();
   }
}