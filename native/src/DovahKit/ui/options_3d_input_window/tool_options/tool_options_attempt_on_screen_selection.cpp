#include "tool_options_attempt_on_screen_selection.h"
#include <QGridLayout>
#include <QLabel>
#include "editor/subsystems/worldinput/tools/_options.h"

namespace worldinput {
   using namespace dovahkit::subsystems::worldinput;
}

namespace {
   template<typename E> void _set_combobox_to_enum(QComboBox* widget, E v) {
      widget->setCurrentIndex(widget->findData((int)v));
   }
}

namespace DK3DToolOptions {
   AttemptOnScreenSelection::AttemptOnScreenSelection(QWidget* parent) : Base(worldinput::id_of_tool<tool_type>(), parent) {
      /*QObject::connect(this, &Base::controlTypeChanged, this, [this]() {
      });*/
      //
      auto* layout = new QGridLayout(this);
      {
         auto*& widget = this->ui.operation;
         widget = new QComboBox(this);

         widget->addItem("Do nothing", (int)worldinput::selection_operation::no_op);
         widget->addItem("Add to selection", (int)worldinput::selection_operation::add);
         widget->addItem("Remove from selection", (int)worldinput::selection_operation::remove);
         widget->addItem("Toggle selected", (int)worldinput::selection_operation::toggle);
         widget->addItem("Replace selection", (int)worldinput::selection_operation::replace);

         layout->addWidget(new QLabel(tr("Operation:"), this), 0, 0);
         layout->addWidget(widget, 0, 1);
      }
      {
         auto*& widget = this->ui.pointer;
         widget = new QComboBox(this);

         widget->addItem("Mouse position", (int)worldinput::pointer_position_type::mouse);
         widget->addItem("Reticle", (int)worldinput::pointer_position_type::reticle);

         layout->addWidget(new QLabel(tr("Pointer:"), this), 1, 0);
         layout->addWidget(widget, 1, 1);
      }
      layout->setContentsMargins({ 0, 0, 0, 0 });
      //
      QObject::connect(this->ui.operation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Base::edited); // signal-to-signal
      QObject::connect(this->ui.pointer,   QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Base::edited); // signal-to-signal
   }

   void AttemptOnScreenSelection::_readOptions(const opaque_option_union& oou) {
      const auto* o = worldinput::tools::option_union::as<tool_type>(oou);
      if (!o)
         return;
      const auto blocker0 = QSignalBlocker(this->ui.operation);
      const auto blocker1 = QSignalBlocker(this->ui.pointer);
      _set_combobox_to_enum(this->ui.operation, o->operation);
      _set_combobox_to_enum(this->ui.pointer,   o->position);
   }
   void AttemptOnScreenSelection::_writeOptions(opaque_option_union& oou) {
      auto* o = worldinput::tools::option_union::as<tool_type>(oou);
      if (!o)
         return;
      o->operation = (worldinput::selection_operation)   this->ui.operation->currentData().toInt();
      o->position  = (worldinput::pointer_position_type) this->ui.pointer->currentData().toInt();
   }
}