#include "save_window.h"
#include <QMessageBox>
#include "../../helpers/filesystem.h"
#include "../../helpers/miscellaneous.h"
#include "../../dovah/files/file_header.h"
#include "../../dovah/files/tes_file_writing/config.h"
#include "../../dovah/files/tes_file_writing/results.h"
#include "../../editor/core.h"
#include "../../dovah/notice_code_list.h"
#include "../main_window.h"

#include "editor/ini/main.h"

ActiveFileSaveDialog::ActiveFileSaveDialog(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   this->ui.game->clear();
   {
      auto* widget = this->ui.game;
      widget->addItem(tr("Skyrim Classic"), (int)dovah::game::skyrim_classic);
      widget->addItem(tr("Skyrim Special"), (int)dovah::game::skyrim_special);
   }
   //
   this->ui.compressionThreshold->setRange(64, std::numeric_limits<decltype(dovah::tes_file_writing::write_config::record_compress_threshold)>::max());
   QObject::connect(this->ui.compressionPolicy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
      this->ui.compressionThreshold->setEnabled(index == 1);
   });
   this->ui.compressionThreshold->setEnabled(this->ui.compressionPolicy->currentIndex() == 1);
   //
   QObject::connect(this->ui.game, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
      this->ui.flagLight->setDisabled(index == 0);
   });
   //
   auto& editor = DovahKitCore::get();
   if (!editor.has_active_file()) {
      this->reject();
      return;
   }
   this->ui.game->setCurrentIndex(editor.get_current_game() == dovah::game::skyrim_special);
   if (editor.active_file_has_name()) {
      this->ui.filename->setText(editor.get_active_file_name());
      //
      auto* header = editor.get_active_file_header();
      if (header) {
         this->ui.flagMaster->setChecked(header->flags & dovah::tes_file_header::flag::master);
         this->ui.flagLight->setChecked(header->flags & dovah::tes_file_header::flag::light);
      }
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
   auto& editor = DovahKitCore::get();
   //
   QString filename_text = this->ui.filename->text();
   std::filesystem::path filename = filename_text.toStdWString();
   if (!editor.active_file_has_name()) {
      if (filename.empty()) {
         QMessageBox::critical(
            this,
            tr("Error", "save error"),
            tr("You must specify a filename.", "save error")
         );
         return;
      }
   }
   if (editor.load_order_has_file(filename, true)) {
      QMessageBox::critical(
         this,
         tr("Error", "save error"),
         tr("The name \"%1\" is already in use by one of this file's dependencies.", "save error").arg(filename_text)
      );
      return;
   }
   auto code = cobb::validate_filename(filename);
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
   if (!cobb::filename_has_extension(filename, { ".esl", ".esm", ".esp" })) {
      QMessageBox::critical(
         this,
         tr("Error", "save error"),
         tr("The filename you entered is invalid. You must use one of the supported file extensions: ESL ESM ESP.", "save error")
      );
      return;
   }
   //
   dovah::game game = (dovah::game)this->ui.game->currentData().toInt();
   std::filesystem::path install_path;
   editor.get_game_path(install_path, game);
   install_path.append("Data");
   editor.set_load_order_folder(install_path); // in case the user never actually loaded a file and is making a file with no masters

   //
   // TODO: if a file with this name exists, pop a confirmation prompt before just overwriting it
   //

   auto config = dovah::tes_file_writing::write_config::for_game(game);
   cobb::edit_bit(config.file_flags, dovah::tes_file_flag::light,  this->ui.flagLight->isChecked());
   cobb::edit_bit(config.file_flags, dovah::tes_file_flag::master, this->ui.flagMaster->isChecked());
   {
      using namespace dovahkit::ini::main;

      config.persistent_refs.add_flag_when_needed      = saving::bApplyRefPersistenceAsNeeded.get_current_value<bool>();
      config.persistent_refs.remove_flag_when_unneeded = saving::bClearRefPersistenceWhenAble.get_current_value<bool>();
   }
   //
   // TODO: We should preserve any flags on the original file, unless they are flags controllable 
   // from this UI, or unless they are flags for things we can't edit (e.g. localized strings).
   //
   switch (this->ui.compressionPolicy->currentIndex()) {
      case 0: config.record_compression = dovah::tes_file_writing::record_compression_policy::never;     break;
      case 1: config.record_compression = dovah::tes_file_writing::record_compression_policy::threshold; break;
      case 2: config.record_compression = dovah::tes_file_writing::record_compression_policy::bethesda;  break;
   }
   config.record_compress_threshold = this->ui.compressionThreshold->value();
   //
   std::vector<dovah::form_stub*> forms_we_cant_save;
   editor.for_each_impossible_to_save_form(config.output_game, [&forms_we_cant_save](dovah::form_stub* stub) {
      forms_we_cant_save.push_back(stub);
      return false;
   });
   if (forms_we_cant_save.size()) {
      //
      // TODO: Show detailed information on the relevant forms
      //
      auto choice = QMessageBox::critical(
         this,
         tr("Warning", "save error"),
         tr("The active file currently contains %1 forms that are not supported in the target game. Not only will these forms not be saved; they will also be deleted from memory if the save operation completes successfully.<br/><br/>Are you sure you still want to convert this file to the selected game?")
            .arg(forms_we_cant_save.size()),
         QMessageBox::YesToAll | QMessageBox::Cancel
      );
      if (choice == QMessageBox::Cancel)
         return;
   }
   //
   dovah::tes_file_writing::write_results results;
   auto success = editor.save_active_file(filename, config, results);
   if (!success) {
      this->handleLastSaveError(results);
      this->reject();
   } else {
      for (auto& warning : results.warnings) {
         if (warning.code == dovah::notice_code::save_complete_but_to_temporary_file) {
            std::string filename;
            if (!warning.relevant_files.empty())
               filename = warning.relevant_files.back();
            QString message = tr("A minor problem occurred: DovahKit was unable to replace the old active file with the newly-written data. Your work has been saved to %1.").arg(filename.c_str());
            QMessageBox::critical(
               this,
               tr("Warning", "save error"),
               message
            );
            break;
         }
      }
      this->accept();
   }
}
void ActiveFileSaveDialog::handleLastSaveError(const dovah::tes_file_writing::write_results& results) {
   using notice_flag = dovah::detailed_notice::flag;
   //
   bool        requires_reload = false;
   bool        temporary_file  = false;
   std::string temporary_name;
   auto& editor = DovahKitCore::get();
   auto& error  = results.error;
   //
   for (auto& warning : results.warnings) {
      if (warning.code == dovah::notice_code::save_complete_but_to_temporary_file) {
         temporary_file = true;
         if (!warning.relevant_files.empty())
            temporary_name = warning.relevant_files.back();
         break;
      }
   }
   if (error.code == dovah::default_notice_code && temporary_file) {
      QMessageBox::critical(
         this,
         tr("Warning", "save warning"),
         tr("The file was successfully saved, but DovahKit was unable to replace the old active file with the newly-written data. Your work has been saved to %1.").arg(temporary_name.c_str())
      );
      return;
   }
   //
   QString message;
   switch (error.code) {
      case dovah::notice_code::unknown_form_type:
         message = tr("One of the forms that needs to be saved is of a type that DovahKit has not yet been programmed to handle.", "write error");
         break;
      case dovah::notice_code::no_active_file:
         message = tr("You did not select an active file, and there is no room in this load order for another file.", "write error");
         break;
      case dovah::notice_code::cannot_save_right_now:
         message = tr("It is not safe to save right now, because DovahKit is currently performing some other operation (e.g. a load or save).", "write error");
         break;
      case dovah::notice_code::no_filename_specified:
         message = tr("The active file is implicit (nameless) and no filename was provided. (Wait, what? How did this happen? We should've made you either provide a name or cancel.)", "write error");
         break;
      case dovah::notice_code::save_complete_but_reopen_failed:
         requires_reload = true;
         message = tr("The file was successfully saved, but could not be reopened for editing after the save. Further editing is no longer possible; you can keep using DovahKit, but all currently loaded data will be unloaded. ", "write error");
         //
         // The mass data unload for this specific error is handled by the call to (editor.abandon_data) at the bottom of this function.
         //
         break;
      case dovah::notice_code::out_of_memory:
         message = tr("An out-of-memory error occurred at some point during the save process, likely while trying to write a compressed record.", "write error");
         break;
      case dovah::notice_code::zlib_memory_error:
         message = tr("A zlib memory error occurred while trying to save a compressed record.", "write error");
         break;
      case dovah::notice_code::zlib_buffer_error:
         message = tr("A zlib buffer error occurred while trying to save a compressed record.", "write error");
         break;
      case dovah::notice_code::forms_out_of_esl_form_id_range:
         message = tr("You cannot convert a file to an ESL if any of its forms have IDs above XX000FFF.", "write error");
         break;
      case dovah::notice_code::file_has_too_many_dependencies:
         message = tr("A file cannot have more than 254 dependencies.", "write error");
         break;
      case dovah::notice_code::load_order_would_overflow_into_lights:
         message = tr("The current load order would not be possible in Skyrim Special. Too many files (besides the active file) are loaded; they are overflowing into the 0xFE slot.", "write error");
         break;
      case dovah::notice_code::load_order_contains_light_files:
         message = tr("The current load order would not be possible in Skyrim Classic. The load order contains ESL files (besides the active file).", "write error");
         break;
      case dovah::notice_code::unsaved_form_cleanup_failed:
         requires_reload = true;
         message = tr("The file was successfully saved, but some forms were lost during the conversion. Internal errors occurred while trying to remove these forms from memory. Further editing is no longer possible; you can keep using DovahKit, but all currently loaded data will be unloaded. ", "write error");
         //
         // The mass data unload for this specific error is handled by the call to (editor.abandon_data) at the bottom of this function.
         //
         break;
      case dovah::notice_code::post_save_none_stub_cleanup_failed:
         requires_reload = true;
         message = tr("The file was successfully saved, but internal errors occurred while trying to clean up information on dangling form-to-form references. Further editing is no longer possible; you can keep using DovahKit, but all currently loaded data will be unloaded. ", "write error");
         //
         // The mass data unload for this specific error is handled by the call to (editor.abandon_data) at the bottom of this function.
         //
         break;
      default:
         message = tr("Unknown error.", "write error");
         break;
   }
   if (temporary_file) {
      message += tr("\r\n\r\nAn additional problem occurred: DovahKit was unable to replace the old active file with the newly-written data. Your work has been saved to %1.").arg(temporary_name.c_str());
   }
   //
   QString text = QString("Unable to save the file. %1").arg(message);
   if (error.flags & notice_flag::has_cause_form) {
      text = QString("Unable to save the file. %1<br/>Form ID: %2<br/>Form type: %3<br/>File offset: %4")
         .arg(message)
         .arg(error.cause_form.fixedID, 8, 16, QChar('0'))
         .arg((int)error.cause_form.type)
         .arg(error.offset);
   } else {
      if (error.flags & notice_flag::has_file_offset)
         text += tr("<br/>File offset: %1").arg(error.offset);
   }
   QMessageBox::critical(
      this,
      tr("Error", "save error"),
      text
   );
   if (requires_reload)
      editor.abandon_data();
}