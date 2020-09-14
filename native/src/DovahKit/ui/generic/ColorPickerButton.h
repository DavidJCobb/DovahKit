#pragma once
#include <QPushButton>

class ColorPickerButton : public QPushButton {
   Q_OBJECT
   public:
      ColorPickerButton(QWidget* parent = nullptr);
      inline QColor color() const noexcept { return this->_color; }
      inline bool hasAlpha() const noexcept { return this->_hasAlpha; }
      void setColor(QColor);
      void setHasAlpha(bool);
      //
   signals:
      void colorChanged();
      //
   protected:
      bool   _hasAlpha = false;
      QColor _color;
      //
      void _updateColor();
};