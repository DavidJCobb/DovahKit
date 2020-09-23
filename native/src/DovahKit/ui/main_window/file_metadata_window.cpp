#include "file_metadata_window.h"
#include "../../editor/core.h"

FileMetadataWindow::FileMetadataWindow(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   auto& editor = DovahKitCore::get();
   if (!editor.has_active_file()) { // caller should check this, tho
      this->reject();
      return;
   }
   this->ui.author->setText(editor.get_active_file_author());
   this->ui.description->setPlainText(editor.get_active_file_description());
   //
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, [this]() {
      this->reject();
   });
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, [this]() {
      this->commit();
      this->accept();
   });
}

void FileMetadataWindow::commit() {
   auto& editor = DovahKitCore::get();
   editor.set_active_file_author(this->ui.author->text());
   editor.set_active_file_description(this->ui.description->toPlainText());
}