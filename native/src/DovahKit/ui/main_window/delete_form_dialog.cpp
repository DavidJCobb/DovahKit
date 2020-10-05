#include "delete_form_dialog.h"
#include "../../dovah/form_stub.h"
#include "../../editor/core.h"

DeleteFormDialog::DeleteFormDialog(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->reject();
   });
}