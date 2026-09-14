#include "./save_window.h"
#include <QMessageBox>
#include "helpers/filesystem.h"
#include "helpers/miscellaneous.h"
#include "dovah/data/game/max_file_version.h"
#include "dovah/data/game.h"
#include "dovah/files/file_header.h"
#include "dovah/files/tes_file_reading/file_loader.h"
#include "dovah/files/tes_file_writing/config.h"
#include "dovah/files/tes_file_writing/results.h"
#include "editor/core.h"
#include "editor/helpers/backend_error_to_string.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/subsystems/message_log/core.h"
#include "../main_window.h"

#include "editor/ini/main.h"

#include "dovah/exceptions/file_save_failed.h"
#include "dovah/exceptions/game_change_failed.h"
#include "dovah/notices/base_form_save_error.h"

#include "editor/helpers/update_active_file_seq_file.h"

ActiveFileSaveDialog::ActiveFileSaveDialog(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   this->ui.game->clear();
   {
      auto* widget = this->ui.game;
      widget->addItem(tr("Skyrim Classic"), (int)dovah::game::skyrim_classic);
      widget->addItem(tr("Skyrim Special"), (int)dovah::game::skyrim_special);
   }
   {
      auto* widget = this->ui.largeRefPolicy;
      widget->clear();
      widget->addItem(tr("Remove", "large ref policy"), (int)dovah::tes_file_writing::large_ref_index_policy::remove);
      widget->addItem(tr("Retain", "large ref policy"), (int)dovah::tes_file_writing::large_ref_index_policy::retain);
      widget->addItem(tr("Update", "large ref policy"), (int)dovah::tes_file_writing::large_ref_index_policy::update);
   }
   {
      auto* widget = this->ui.refPersistence;
      widget->clear();
      widget->addItem(tr("Unchanged",  "ref persistence policy"), 0b00);
      widget->addItem(tr("Update",     "ref persistence policy"), 0b11);
      widget->addItem(tr("Apply Only", "ref persistence policy"), 0b01);
      widget->addItem(tr("Clear Only", "ref persistence policy"), 0b10);

      using namespace dovahkit::ini::main;
      bool apply = saving::bApplyRefPersistenceAsNeeded.get_current_value<bool>();
      bool clear = saving::bClearRefPersistenceWhenAble.get_current_value<bool>();

      int value = 0;
      if (apply)
         value |= 1;
      if (clear)
         value |= 2;

      auto i = widget->findData(value);
      if (i >= 0)
         widget->setCurrentIndex(i);
   }
   //
   this->ui.compressionThreshold->setRange(64, std::numeric_limits<decltype(dovah::tes_file_writing::write_config::record_compress_threshold)>::max());
   QObject::connect(this->ui.compressionPolicy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
      this->ui.compressionThreshold->setEnabled(index == 1);
   });
   this->ui.compressionThreshold->setEnabled(this->ui.compressionPolicy->currentIndex() == 1);
   //
   QObject::connect(this->ui.game, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
      const auto game = (dovah::game)this->ui.game->currentData().toInt();

      this->ui.flagLight->setDisabled(game == dovah::game::skyrim_classic);
      this->ui.flagUse1Point71FormIDSpace->setVisible(game == dovah::game::skyrim_special);
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

namespace {
   QString _stringify_error_code(dovah::exceptions::file_save_failed::error_code code) {
      using error_code = decltype(code);
      switch (code) {
         case error_code::forms_out_of_esl_form_id_range:
            return QObject::tr("You cannot convert a file to an ESL if any of its forms have IDs above XX000FFF.", "write error");
         case error_code::file_has_too_many_dependencies:
            return QObject::tr("A file cannot have more than 254 dependencies.", "write error");
         case error_code::no_active_file:
            return QObject::tr("You did not select an active file, and there is no room in this load order for another file.", "write error");
         case error_code::save_or_load_already_in_progress:
            return QObject::tr("It is not safe to save right now, because DovahKit is currently performing some other operation (e.g. a load or save).", "write error");
         case error_code::no_filename_specified:
            return QObject::tr("The active file is implicit (nameless) and no filename was provided. (Wait, what? How did this happen? We should've made you either provide a name or cancel.)", "write error");
         case error_code::save_complete_but_reopen_failed:
            return QObject::tr("The file was successfully saved, but could not be reopened for editing after the save. Further editing is no longer possible; you can keep using DovahKit, but all currently loaded data will be unloaded. ", "write error");
         case error_code::out_of_memory:
            return QObject::tr("An out-of-memory error occurred at some point during the save process, likely while trying to write a compressed record.", "write error");
         case error_code::zlib_memory_error:
            return QObject::tr("A zlib memory error occurred while trying to save a compressed record.", "write error");
         case error_code::zlib_buffer_error:
            return QObject::tr("A zlib buffer error occurred while trying to save a compressed record.", "write error");
         case error_code::unsaved_form_cleanup_failed:
            return QObject::tr("The file was successfully saved, but some forms were lost during the conversion. Internal errors occurred while trying to remove these forms from memory. Further editing is no longer possible; you can keep using DovahKit, but all currently loaded data will be unloaded. ", "write error");
         case error_code::post_save_none_stub_cleanup_failed:
            return QObject::tr("The file was successfully saved, but internal errors occurred while trying to clean up information on dangling form-to-form references. Further editing is no longer possible; you can keep using DovahKit, but all currently loaded data will be unloaded. ", "write error");
         case error_code::unimplemented_form_type:
            return QObject::tr("One of the edited forms is of a form type that DovahKit doesn't know how to save. (Wait, what? How did this happen?) ", "write error");
         case error_code::desired_file_version_does_not_support_co_opting_the_hardcoded_form_id_range:
            return QObject::tr("The active file defines forms that fall within the hardcoded form ID range [xx000001, xx0007FF]. The desired save version doesn't support this.", "write error");
      }
      return QObject::tr("An unknown problem occurred while trying to save this file.", "write error");
   }
}

std::filesystem::path ActiveFileSaveDialog::_get_target_filename() {
   auto& editor = DovahKitCore::get();

   QString filename_text = this->ui.filename->text();
   std::filesystem::path filename = filename_text.toStdWString();
   if (!editor.active_file_has_name()) {
      if (filename.empty()) {
         QMessageBox::critical(
            this,
            tr("Error", "save error"),
            tr("You must specify a filename.", "save error")
         );
         return {};
      }
   }
   if (editor.load_order_has_file(filename, true)) {
      QMessageBox::critical(
         this,
         tr("Error", "save error"),
         tr("The name \"%1\" is already in use by one of this file's dependencies.", "save error").arg(filename_text)
      );
      return {};
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
      return {};
   }
   if (!cobb::filename_has_extension(filename, { ".esl", ".esm", ".esp" })) {
      QMessageBox::critical(
         this,
         tr("Error", "save error"),
         tr("The filename you entered is invalid. You must use one of the supported file extensions: ESL ESM ESP.", "save error")
      );
      return {};
   }

   //
   // TODO: if a file with this name exists, pop a confirmation prompt before just overwriting it
   //

   return filename;
}

void ActiveFileSaveDialog::_force_current_editor_base_path(dovah::game game) {
   auto& editor = DovahKitCore::get();

   std::filesystem::path install_path;
   editor.get_game_path(install_path, game);
   install_path.append("Data");
   editor.set_load_order_folder(install_path);
}

bool ActiveFileSaveDialog::_check_cross_game_masters_exist(dovah::game game) {
   std::filesystem::path dst_path;
   auto& editor = DovahKitCore::get();
   if (!editor.get_game_path(dst_path, game))
      return true;
   dst_path /= "Data\\";

   std::vector<std::string> missing_files;
   {
      auto  files         = editor.get_loaded_files();
      auto* active_header = editor.get_active_file_header();
      for (auto* file : files) {
         auto filename      = file->get_filename();
         auto matching_file = dst_path / filename;
         if (!std::filesystem::exists(matching_file))
            missing_files.push_back(std::move(filename));
      }
   }
   if (missing_files.empty())
      return true;

   QString file_list = "<ul>";
   for (auto& filename : missing_files) {
      file_list += "<li>";
      file_list += QString::fromStdString(filename);
      file_list += "</li>";
      filename.clear();
   }
   missing_files.clear();
   file_list += "</ul>";
   
   auto message = QMessageBox(
      QMessageBox::Warning,
      tr("Warning"),
      tr(
         "<p>You are converting the active file across games, but some of the active file's masters "
         "don't exist in the destination game's Data directory. Specifically, the following files "
         "appear to be missing:</p>"
         "%1"
         "<p>Are you sure you want to proceed and save a converted file?</p>"
      ).arg(file_list),
      QMessageBox::StandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No),
      this
   );
   message.setDefaultButton(QMessageBox::StandardButton::No);

   message.exec();
   if (message.clickedButton() != message.button(QMessageBox::StandardButton::Yes))
      return false;

   return true;
}

bool ActiveFileSaveDialog::_enforce_form_id_ranges(bool allow_bees, bool allow_non_esl) {
   auto& editor = DovahKitCore::get();

   size_t forms_total            = 0;
   size_t forms_in_bees_range    = 0;
   size_t forms_not_in_esl_range = 0;
   editor.for_each_form([&editor, &forms_total , &forms_in_bees_range, &forms_not_in_esl_range](dovah::form_stub* stub) -> bool {
      if (!editor.is_form_defined_in_active_file(stub))
         return false;
      if (stub->is_injected())
         return false;
      ++forms_total;

      uint32_t form_id = stub->formID & 0x00FFFFFF;
      if (form_id & 0x00FFF000) {
         ++forms_not_in_esl_range;
      }
      {
         uint32_t mask = 0x00FFFFFF;
         if ((form_id >> 0x18) == 0xFE)
            mask = 0x00000FFF;
         if ((form_id & mask) <= dovah::max_hardcoded_form_id) {
            ++forms_in_bees_range;
         }
      }
      return false;
   });

   if (!allow_non_esl) {
      size_t maximum_total_count = 0xFFF + 1;
      if (!allow_bees)
         maximum_total_count -= dovah::max_hardcoded_form_id + 1;
      if (editor.get_loaded_files().size() == 1)
         maximum_total_count -= 1; // Can't use xxyyy000 if no masters, as that works out to record ID 00000000.
      if (forms_total > maximum_total_count) {
         QString format;
         if (allow_bees) {
            format = tr("This file contains %1 forms. An ESL file that doesn't use extended form IDs can only contain %2 forms.");
         } else {
            format = tr("This file contains %1 forms. An ESL file using extended form IDs can only contain %2 forms.");
         }
         QMessageBox::critical(
            this,
            tr("Error", "save error"),
            format.arg(forms_total).arg(maximum_total_count)
         );
         return false;
      }
      if (forms_not_in_esl_range > 0) {
         auto message = QMessageBox(
            QMessageBox::Warning,
            tr("Warning"),
            tr(
               "This file contains %1 forms out of %3 allowed, of which %2 are outside the range of form IDs allowed in "
               "an ESL file. Would you like to try compacting form IDs before saving?"
            ).arg(forms_total).arg(forms_not_in_esl_range).arg(maximum_total_count),
            QMessageBox::StandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No),
            this
         );
         message.exec();
         if (message.clickedButton() != message.button(QMessageBox::StandardButton::Yes))
            return false;

         if (!editor.try_compact_form_ids(true, allow_bees, true)) {
            // should be impossible
            QMessageBox::critical(this,
               tr("Error", "saving"),
               tr("Unable to compact form IDs. Please report this to DovahKit's developer, and provide a saved copy of your current file.\n\nYour file has not been saved."),
               QMessageBox::Ok
            );
            return false;
         }
         forms_in_bees_range    = 0;
         forms_not_in_esl_range = 0;
      }
   }
   if (!allow_bees && forms_in_bees_range > 0) {
      auto message = QMessageBox(
         QMessageBox::Warning,
         tr("Warning"),
         tr(
            "This file contains %1 forms whose IDs are in the extended form ID range. Would you like to try renumbering "
            "those forms in bulk before saving?"
         ).arg(forms_in_bees_range),
         QMessageBox::StandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No),
         this
      );
      message.exec();
      if (message.clickedButton() != message.button(QMessageBox::StandardButton::Yes))
         return false;

      if (!editor.try_move_form_ids_out_of_hardcoded_ambiguous_range()) {
         // should be impossible
         QMessageBox::critical(this,
            tr("Error", "saving"),
            tr("Unable to move form IDs out of the range [xxyyy000, xxyyy7FF]. Please report this to DovahKit's developer, and provide a saved copy of your current file.\n\nYour file has not been saved."),
            QMessageBox::Ok
         );
         return false;
      }
      forms_in_bees_range = 0;
   }
   return true;
}
bool ActiveFileSaveDialog::_enforce_cross_game_form_loss_is_deliberate(dovah::game game) {
   auto& editor = DovahKitCore::get();

   std::vector<dovah::form_stub*> forms_we_cant_save;
   editor.for_each_impossible_to_save_form(game, [&forms_we_cant_save](dovah::form_stub* stub) {
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
      if (choice == QMessageBox::Cancel) {
         return false;
      }
   }
   return true;
}
bool ActiveFileSaveDialog::_enforce_esl_interiors_are_deliberate() {
   auto& editor = DovahKitCore::get();

   size_t defined_cell_count = 0;
   editor.for_each_form_of_type(dovah::form_type::cell, [&editor, &defined_cell_count](dovah::form_stub* stub) -> bool {
      if (stub->get_parent_form())
         return false;
      if (!editor.is_form_defined_in_active_file(stub))
         return false;
      ++defined_cell_count;
      return false;
   });
   if (defined_cell_count > 0) {
      auto message = QMessageBox(
         QMessageBox::Warning,
         tr("Warning"),
         tr(
            "<p>This file defines %1 interior cell(s). Saving it as an ESL may cause game instability:</p>"
            "<ul>"
            "<li><p>If you reload a save, Skyrim Special Edition won't reload references in an interior "
            "cell created by an ESL.</p></li>"
            "<li><p>If an interior cell is defined by an ESL and then patched by another mod, the cell "
            "will break completely.</p></li>"
            "</ul>"
            "<p>The \"SSE Engine Fixes\" mod (from version 7.0.14 onward) fixes these issues and makes "
            "ESL-defined interior cells safe. If that's acceptable, you will need to remember to inform "
            "your users that it's a requirement for your mod.</p>"
            "<p>Are you sure you want to proceed and save this file as an ESL?</p>"
         ).arg(defined_cell_count),
         QMessageBox::StandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No),
         this
      );
      message.setDefaultButton(QMessageBox::StandardButton::No);

      message.exec();
      if (message.clickedButton() != message.button(QMessageBox::StandardButton::Yes))
         return false;
   }
   return true;
}

void ActiveFileSaveDialog::commit() {
   auto& editor = DovahKitCore::get();
   //
   auto filename = this->_get_target_filename();
   if (filename.empty())
      return;

   dovah::game game_prior = editor.get_current_game();
   dovah::game game       = (dovah::game)this->ui.game->currentData().toInt();
   this->_force_current_editor_base_path(game);

   auto config = dovah::tes_file_writing::write_config::for_game(game);
   cobb::edit_bit(config.file_flags, dovah::tes_file_flag::light,  game != dovah::game::skyrim_classic && this->ui.flagLight->isChecked());
   cobb::edit_bit(config.file_flags, dovah::tes_file_flag::master, this->ui.flagMaster->isChecked());
   if (game == dovah::game::skyrim_special) {
      if (this->ui.flagUse1Point71FormIDSpace->isChecked()) {
         config.use_file_version = dovah::game_feature_support::max_file_version(game);
      } else {
         config.use_file_version = 1.70F;
      }
   }

   if (game != game_prior)
      if (!_check_cross_game_masters_exist(game))
         return;

   if (!this->_enforce_form_id_ranges(
      this->ui.flagUse1Point71FormIDSpace->isChecked(),
      (config.file_flags & dovah::tes_file_flag::light) == 0
   )) {
      return;
   }
   {
      int  value = 0b11;
      auto data  = this->ui.refPersistence->currentData();
      if (!data.isNull()) {
         value = data.toInt();
      }
      config.persistent_refs.add_flag_when_needed      = value & 1;
      config.persistent_refs.remove_flag_when_unneeded = value & 2;
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

   config.large_refs = (dovah::tes_file_writing::large_ref_index_policy) this->ui.largeRefPolicy->currentData().toInt();
   
   if (!this->_enforce_cross_game_form_loss_is_deliberate(config.output_game))
      return;

   if (config.file_flags & dovah::tes_file_flag::light) {
      if (!this->_enforce_esl_interiors_are_deliberate())
         return;
   }

   auto& logging = dovahkit::subsystems::message_log::core::get_or_create();
   
   dovah::tes_file_writing::write_results results;
   try {
      editor.save_active_file(filename, config, results);
   } catch (const dovah::exceptions::game_change_failed& ex) {
      this->_force_current_editor_base_path(game_prior);

      using exception  = std::decay_t<decltype(ex)>;
      using error_code = exception::error_code;

      QString target_game;
      switch (ex.to) {
         case dovah::game::skyrim_classic:
            target_game = tr("Skyrim Classic");
            break;
         case dovah::game::skyrim_special:
            target_game = tr("Skyrim Special");
            break;
      }

      QString message;
      switch (ex.code) {
         case error_code::load_order_would_overflow_into_lights:
            message = tr("The current load order would not be possible in %1. Too many files (besides the active file) are loaded; they are overflowing into the 0xFE slot.", "write error")
               .arg(target_game);
            break;
         case error_code::load_order_contains_light_files:
            message = tr("The current load order would not be possible in %1. The load order contains ESL files (besides the active file).", "write error")
               .arg(target_game);
            break;
         case error_code::active_file_co_opts_the_hardcoded_form_id_range:
            message = tr(
               "The active file would not be possible in %1. The active file contains form IDs in the range [xx000001, xx0007FF]. In %1, "
               "form IDs in this range are reserved for hardcoded forms regardless of the load order prefix they're saved with. To convert "
               "your file for the target game, you must first renumber all such forms to use valid form IDs outside of that range.",
               "write error"
            ).arg(target_game);
            break;
         default:
            message = tr("An unknown problem occurred while trying to save this file for %1.", "write error").arg(target_game);
            break;
      }

      QString text = QString("Unable to save the file. %1").arg(message);
      //
      logging.addLogItem(ui::types::log_item(text, ui::types::log_item_type::error, ui::types::log_item_context::file_save));
      //
      QMessageBox::critical(
         this,
         tr("Error", "save error"),
         text
      );
      this->reject();
      return;
   } catch (const dovah::exceptions::file_save_failed& ex) {
      this->_force_current_editor_base_path(game_prior);

      using exception  = std::decay_t<decltype(ex)>;
      using error_code = exception::error_code;

      bool    requires_reload = false;
      QString message = _stringify_error_code(ex.code);
      switch (ex.code) {
         case error_code::save_complete_but_reopen_failed:
         case error_code::unsaved_form_cleanup_failed:
         case error_code::post_save_none_stub_cleanup_failed:
            requires_reload = true;
            //
            // The mass data unload for this specific error is handled by the call to (editor.abandon_data) at the bottom of this function.
            //
            break;
      }
      if (ex.code == error_code::form_save_failed) {
         if (ex.details.form_save_error) {
            message = editor_helpers::backend_error_to_string(*ex.details.form_save_error);
         } else {
            message = tr("An unknown problem occurred while trying to save one of the forms in this file.", "write error");
         }
      } else if (ex.code == error_code::unimplemented_form_type) {
         if (ex.details.unimplemented_form) {
            message = tr("Form %1 is of a type that DovahKit doesn't know how to save.", "write error")
               .arg(editor_helpers::form_identifiers_to_string(ex.details.unimplemented_form));
         }
      }

      QString text = QString("Unable to save the file. %1").arg(message);
      //
      logging.addLogItem(ui::types::log_item(text, ui::types::log_item_type::error, ui::types::log_item_context::file_save));
      //
      QMessageBox::critical(
         this,
         tr("Error", "save error"),
         text
      );
      if (requires_reload) {
         editor.abandon_data();
      }

      this->reject();
      return;
   }

   auto seq_result = editor_helpers::update_active_file_seq_file(filename, this->ui.deleteEmptySeq->isChecked());

   if (results.saved_to_temporary_file) {
      QString message = tr("A minor problem occurred: DovahKit was unable to replace the old active file with the newly-written data. Your work has been saved to %1.").arg(results.filename.c_str());
      //
      logging.addLogItem(ui::types::log_item(message, ui::types::log_item_type::warning, ui::types::log_item_context::file_save));
      //
      QMessageBox::critical(
         this,
         tr("Warning", "save warning"),
         message
      );
   }
   if (seq_result != editor_helpers::update_active_file_seq_file_result::success) {
      QString message;
      switch (seq_result) {
         using enum editor_helpers::update_active_file_seq_file_result;
         case unable_to_delete:
            message = tr("The updated SEQ file would be empty. For some reason, DovahKit could not delete it.");
            break;
         case unable_to_write_temporary:
            message = tr("For some reason, DovahKit was unable to write updated SEQ file to a temporary file (with the intention of replacing the original file after successful writing).");
            break;
         case unable_to_relocate:
            message = tr("DovahKit wrote the SEQ data to a temporary file, with the intention of replacing any existing SEQ file only after all data was successfully written. For some reason, the replacmenet failed.");
            break;
      }
      message = tr("A minor problem occurred while updating the SEQ file. %1").arg(message);
      //
      logging.addLogItem(ui::types::log_item(message, ui::types::log_item_type::warning, ui::types::log_item_context::file_save));
      //
      QMessageBox::critical(
         this,
         tr("Warning", "save warning"),
         message
      );
   }

   this->accept();
}