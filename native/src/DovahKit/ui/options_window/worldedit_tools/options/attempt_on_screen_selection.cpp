#include "./attempt_on_screen_selection.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>

namespace dovahkit::ui::worldedit::tools {
   attempt_on_screen_selection::attempt_on_screen_selection(QWidget* parent) : QWidget(parent) {
      auto* layout = new QGridLayout(this);
      layout->setContentsMargins(0, 0, 0, 0);
      this->setLayout(layout);

      {
         auto* label = new QLabel(tr("Selection operation:"), this);
         layout->addWidget(label, 0, 0);
      }
      this->_subwidgets.operation = new QComboBox(this);
      layout->addWidget(this->_subwidgets.operation, 0, 1);

      {
         using values = decltype(options_type::operation);

         auto* widget = this->_subwidgets.operation;
         widget->addItem(tr("Do nothing"), (int)values::no_op);
         widget->addItem(tr("Add to selection"), (int)values::add);
         widget->addItem(tr("Remove from selection"), (int)values::remove);
         widget->addItem(tr("Toggle selected"), (int)values::toggle);
         widget->addItem(tr("Replace selection"), (int)values::replace);

         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
            auto v = this->_subwidgets.operation->currentData().toInt();
            this->_state.current_options.operation = (values)v;
         });
      }
   }

   void attempt_on_screen_selection::set_options(const options_type& v) {
      this->_state.current_options = v;
      {
         using values = decltype(options_type::operation);

         auto* widget = this->_subwidgets.operation;
         auto  i      = widget->findData((int)v.operation);

         const auto blocker = QSignalBlocker(widget);

         if (i < 0) {
            this->_state.current_options.operation = values::no_op;
            widget->setCurrentIndex(0);
         } else {
            widget->setCurrentIndex(i);
         }
      }
   }
}