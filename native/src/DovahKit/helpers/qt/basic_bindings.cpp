#include "basic_bindings.h"

namespace cobb::qt {
   void bind(QCheckBox* widget, bool& target) {
      widget->setChecked(target);
      QObject::connect(widget, &QCheckBox::stateChanged, widget, [&target](int state) {
         target = state == Qt::CheckState::Checked;
      });
   }
   void unbind(QCheckBox* widget) {
      QObject::disconnect(widget, &QCheckBox::stateChanged, widget, nullptr);
   }

   void bind(QLineEdit* widget, std::string& target) {
      widget->setText(QString::fromUtf8(target.c_str()));
      QObject::connect(widget, &QLineEdit::textChanged, widget, [&target](const QString& value) {
         target = value.toUtf8().data();
      });
   }
   void unbind(QLineEdit* widget) {
      QObject::disconnect(widget, &QLineEdit::textChanged, widget, nullptr);
   }
}