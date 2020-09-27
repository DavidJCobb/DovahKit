#include "TESFilePicker.h"

TESFilePicker::TESFilePicker(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   //
   QObject::connect(this->ui.text, &QLineEdit::textChanged, this, &TESFilePicker::currentPathChanged);
   QObject::connect(this->ui.text, &QLineEdit::textEdited,  this, &TESFilePicker::currentPathEdited);
   QObject::connect(this->ui.buttonClear, &QPushButton::clicked, this, [this]() {
      this->ui.text->setText("");
      emit this->currentPathEdited();
   });
}
QString TESFilePicker::currentPath() const noexcept {
   return this->ui.text->text();
}
void TESFilePicker::setCurrentPath(const QString& to) {
   this->ui.text->setText(to);
}
void TESFilePicker::setCurrentPath(const char* to) {
   this->ui.text->setText(to);
}