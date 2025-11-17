#include "./FormSubdialogIdleNewGraph.h"
#include <QMessageBox>

FormSubdialogIdleNewGraph::FormSubdialogIdleNewGraph(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   QObject::connect(this->ui.path, &DKGameFilePicker::valueChanged, this, &FormSubdialogIdleNewGraph::_update_ok_button_enable_state);
   this->_update_ok_button_enable_state();

   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, [this]() {
      auto path = this->path();
      if (!path.contains(".hkx", Qt::CaseInsensitive)) {
         QMessageBox::critical(
            this,
            tr("Invalid behavior graph path"),
            tr("The specified path must end in \".hkx\" (case-insensitive).")
         );
         return;
      }
      auto i = path.indexOf("animations", Qt::CaseInsensitive);
      if (i > 0) { // intentionally > 0 rather than > -1
         QMessageBox::critical(
            this,
            tr("Invalid behavior graph path"),
            tr(
               "Paths that don't contain \".hkx\", but do contain, and do not start with, the "
               "case-insensitive word \"Animations\", will be modified by the Creation Kit and "
               "won't lead to where you expect. (The CK chops off the word \"Animations\" and "
               "the character just before it, and tacks on the suffix \"\\Behaviors\\0_Master.hkx\".)"
            )
         );
         return;
      }

      this->accept();
   });
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