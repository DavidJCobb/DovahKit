#include "save_window.h"
#include <QMessageBox>
#include "../../helpers/filesystem.h"
#include "../../editor/core.h"
#include "../main_window.h"

ActiveFileSaveDialog::ActiveFileSaveDialog(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   auto& editor = DovahKitCore::get();
   if (!editor.has_active_file()) {
      this->reject();
      return;
   }
   if (editor.active_file_has_name()) {
      this->ui.filename->setDisabled(true);
      this->ui.filename->setReadOnly(true);
      this->ui.filename->setText(editor.get_active_file_name());
   }
   editor.for_each_load_order_filename([this](std::filesystem::path filename, bool is_active_file) {
      if (is_active_file)
         return false;
      auto item = new QListWidgetItem(QString::fromStdWString(filename.wstring()));
      this->ui.dependencies->addItem(item);
      return false;
   });
   //
   // TODO: as filename input changes, disable checkboxes where appropriate
   //
   //
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, [this]() {
      this->reject();
   });
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, [this]() {
      this->commit();
   });
}

void ActiveFileSaveDialog::commit() {
   std::filesystem::path fallback_filename;
   //
   auto& editor = DovahKitCore::get();
   if (!editor.active_file_has_name()) {
      auto text = this->ui.filename->text();
      if (text.isEmpty()) {
         QMessageBox::critical(
            this,
            tr("Error", "save error"),
            tr("You must specify a filename.", "save error")
         );
         return;
      }
      fallback_filename = text.toStdWString();
      if (editor.load_order_index_of_file(fallback_filename) != dovah::file_load_order::invalid_load_prefix) {
         QMessageBox::critical(
            this,
            tr("Error", "save error"),
            tr("The name \"%1\" is already in use by one of this file's dependencies.", "save error").arg(text)
         );
         return;
      }
      //
      auto code = cobb::validate_filename(fallback_filename);
      if (code != cobb::filename_validation_result::valid) {
         QString error;
         switch (code) {
            case cobb::filename_validation_result::missing:
               error = tr("You can't save a nameless file with just an extension.", "save filename error");
               break;
            case cobb::filename_validation_result::is_a_path:
               error = tr("You cannot specify paths.", "save filename error");
               break;
            case cobb::filename_validation_result::is_current_or_parent_directory:
               error = tr("You entered a relative directory name.", "save filename error");
               break;
            case cobb::filename_validation_result::windows_device_name:
               error = tr("That filename is one of the \"device file\" names that Windows has kept reserved for backward compatibility since 1970. The operating system won't allow you to use it.", "save filename error");
               break;
            case cobb::filename_validation_result::illegal_character:
               error = tr("You used a symbol that isn't allowed in filenames.", "save filename error");
               break;
         }
         QMessageBox::critical(
            this,
            tr("Error", "save error"),
            tr("The filename you entered is invalid. %1", "save error").arg(error)
         );
         return;
      }
      //
      if (!cobb::filename_has_extension(fallback_filename, { ".esl", ".esm", ".esp" })) {
         QMessageBox::critical(
            this,
            tr("Error", "save error"),
            tr("The filename you entered is invalid. You must use one of the supported file extensions: ESL ESM ESP.", "save error")
         );
         return;
      }
   }
   //
   std::filesystem::path install_path;
   editor.get_game_path(install_path);
   install_path.append("Data");
   editor.set_load_order_folder(install_path); // in case the user never actually loaded a file and is making a file with no masters

   //
   // TODO: if a file with this name exists, pop a confirmation prompt before just overwriting it
   //

   //
   auto result = editor.save_active_file(fallback_filename);
   if (!result) {
      this->reportLastSaveError();
      this->reject();
   } else {
      this->accept();
   }
}
void ActiveFileSaveDialog::reportLastSaveError() {
   auto& editor = DovahKitCore::get();
   auto& error  = editor.get_last_write_error();
   QString message;
   switch (error.code) {
      case dovah::file_write_error::error_code::unknown_form_type:
         message = tr("One of the forms that needs to be saved is of a type that DovahKit has not yet been programmed to handle.", "write error");
         break;
      case dovah::file_write_error::error_code::no_active_file:
         message = tr("You did not select an active file, and there is no room in this load order for another file.", "write error");
         break;
      case dovah::file_write_error::error_code::cannot_save_right_now:
         message = tr("It is not safe to save right now, because DovahKit is currently performing some other operation (e.g. a load or save).", "write error");
         break;
      case dovah::file_write_error::error_code::no_filename_specified:
         message = tr("The active file is implicit (nameless) and no filename was provided. (Wait, what? How did this happen? We should've made you either provide a name or cancel.)", "write error");
         break;
      default:
         message = tr("Unknown error.", "write error");
         break;
   }
   QString text;
   if (error.formID) {
      text = QString("Unable to save the file. %1<br/>Form ID: %2<br/>Form type: %3<br/>File offset: %4")
         .arg(message)
         .arg(error.formID)
         .arg(error.form_type)
         .arg(error.file_offset);
   } else {
      text = QString("Unable to save the file. %1<br/>File offset: %2")
         .arg(message)
         .arg(error.file_offset);
   }
   QMessageBox::critical(
      this,
      tr("Error", "save error"),
      text
   );
}