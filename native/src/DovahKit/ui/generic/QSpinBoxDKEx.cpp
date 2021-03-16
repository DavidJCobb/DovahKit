#include "QSpinBoxDKEx.h"

void QSpinBoxDKEx::setValidateHandler(validate_handler_t handler) noexcept {
   this->handlers.validate = handler;
}

QValidator::State QSpinBoxDKEx::validate(QString& text, int& pos) const {
   auto result = QSpinBox::validate(text, pos);
   if (result == QValidator::State::Invalid)
      return result;
   if (auto hnd = this->handlers.validate)
      return (hnd)(*this, text, pos);
   return result;
}