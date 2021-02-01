#pragma once
#include <QPushButton>

namespace dovah {
   class form_stub;
}

class RefPickerButton : public QPushButton {
   Q_OBJECT
   public:
      RefPickerButton(QWidget* parent = nullptr);
      inline dovah::form_stub* value() const noexcept { return this->_stub; }
      //
   signals:
      void valueChanged(dovah::form_stub*);
      //
   public slots:
      void setValue(dovah::form_stub*);
      //
   protected:
      dovah::form_stub* _stub = nullptr;
      QString _placeholder;
      //
      void _updateText();
};