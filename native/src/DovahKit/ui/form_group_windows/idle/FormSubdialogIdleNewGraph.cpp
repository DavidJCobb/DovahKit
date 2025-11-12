#include "./FormSubdialogIdleNewGraph.h"

FormSubdialogIdleNewGraph::FormSubdialogIdleNewGraph(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   QObject::connect(this->ui.path, &DKGameFilePicker::valueChanged, this, &FormSubdialogIdleNewGraph::_update_ok_button_enable_state);
   this->_update_ok_button_enable_state();

   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
}

QString FormSubdialogIdleNewGraph::path() const {
   auto path = this->ui.path->value();
   return path.lexically_relative("data\\meshes\\").to_string();
}

void FormSubdialogIdleNewGraph::_update_ok_button_enable_state() {
   if (this->ui.path->value().empty()) {
      this->ui.buttonOK->setEnabled(false);
      return;
   }
   this->ui.buttonOK->setEnabled(true);
}