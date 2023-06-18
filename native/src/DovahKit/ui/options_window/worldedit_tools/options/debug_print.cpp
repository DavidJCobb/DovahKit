#include "./debug_print.h"
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>

namespace dovahkit::ui::worldedit::tools {
   debug_print::debug_print(QWidget* parent) : QWidget(parent) {
      auto* layout = new QGridLayout(this);
      layout->setContentsMargins(0, 0, 0, 0);
      this->setLayout(layout);

      {
         auto* label = new QLabel(tr("Text:"), this);
         layout->addWidget(label, 0, 0);
      }
      this->_subwidgets.text = new QLineEdit(this);
      layout->addWidget(this->_subwidgets.text, 0, 1);

      QObject::connect(this->_subwidgets.text, &QLineEdit::textChanged, this, [this]() {
         this->_state.current_options.text = this->_subwidgets.text->text().toStdString();
      });
   }

   void debug_print::set_options(const options_type& v) {
      this->_state.current_options = v;

      const auto blocker = QSignalBlocker(this->_subwidgets.text);
      this->_subwidgets.text->setText(QString::fromStdString(v.text));
   }
}