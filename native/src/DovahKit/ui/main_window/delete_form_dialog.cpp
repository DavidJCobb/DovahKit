#include "delete_form_dialog.h"
#include "../../dovah/form_stub.h"
#include "../../editor/core.h"
#include "delete_form_dialog/delete_form_table.h"

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
void DeleteFormDialog::updateFromDeletionRequest(const dovah::form_deletion_request& request) {
   auto* model_delete = this->ui.formsPendingDelete->unwrappedModel();
   if (!model_delete)
      return;
   model_delete->clear();
   model_delete->insertDeletions(request);
   //
   auto* model_users = this->ui.formsPendingUpdate->unwrappedModel();
   if (!model_users)
      return;
   model_users->clear();
   model_users->insertUsers(request);
}