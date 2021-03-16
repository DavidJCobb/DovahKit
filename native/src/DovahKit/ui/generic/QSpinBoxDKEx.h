#pragma once
#include <functional>
#include <QSpinBox>

class QSpinBoxDKEx : public QSpinBox {
   public:
      using QSpinBox::QSpinBox;
      using validate_handler_t = std::function<QValidator::State(const QSpinBoxDKEx&, QString& input, int& pos)>;

      void setValidateHandler(validate_handler_t) noexcept; // runs after the default behavior, and only if the default behavior doesn't flag the value as Invalid

   protected:
      struct {
         validate_handler_t validate;
      } handlers;

      virtual QValidator::State validate(QString& text, int& pos) const override;
};