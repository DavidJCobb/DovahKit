#pragma once
#include <filesystem>
#include <unordered_map>
#include <QDialog>
#include <QObject>
#include "../dovah/core.h"
#include "../dovah/files/file_load_order.h"

namespace dovah {
   class  bsa_archived_file;
   class  compiled_papyrus_script;
   struct detailed_notice;
   class  form_deletion_request;
   class  form_stub;
   struct localized_string;
   struct tes_file_header;
   namespace tes_file_reading {
      class file_loader;
      class read_results;
   }
   namespace tes_file_writing {
      struct write_config;
      class  write_results;
   }
}
namespace DovahKitEditorInternals {
   class load_task;
}

class AbstractFormEditDialog;
class DKBSACollectionModelBackend;
class FormUseInfoDialog;

class DovahKitCore : public QObject {
   Q_OBJECT
   friend class DovahKitEditorInternals::load_task;
   friend void open_use_info_dialog_for_form(dovah::form_stub*, QWidget* parent);
   friend void open_edit_dialog_for_form(dovah::form_stub*, QWidget* parent);
   public:
      using form_type_t    = dovah::form_type_t;
      using bare_form_id_t = dovah::bare_form_id_t;
      struct file_load_stats {
         uint8_t  file_count   = 0;
         uint32_t microseconds = 0;
         uint32_t milliseconds = 0;
      };
      //
      static DovahKitCore& get() {
         static DovahKitCore instance;
         return instance;
      }
      DovahKitCore();
      ~DovahKitCore();
      //
      DovahKitCore(const DovahKitCore&) = delete; // no copy
      DovahKitCore(DovahKitCore&&) = delete;      // no move
      DovahKitCore& operator=(const DovahKitCore&) = delete; // no copy
      DovahKitCore& operator=(DovahKitCore&&) = delete;      // no move
      //
   protected:
      dovah::file_load_order* load_order = nullptr; // created in constructor
      bool    loaded  = false;
      bool    loading = false;
      QThread* async_loader = nullptr;
      std::string encoding;
      std::string language_name;
      bool com_is_ready = false; // is COM set up for the main thread?
      DKBSACollectionModelBackend* bsa_browse_backend = nullptr;
      //
      std::unordered_map<bare_form_id_t, QDialog*> extant_form_edit_dialogs;
      std::unordered_map<bare_form_id_t, QDialog*> extant_use_info_dialogs;
      //
      void _configure_load_order();
      //
   signals:
      void dataAbandonImminent(); // we are about to abandon all forms; ditch your pointers or risk memory corruption
      void dataAbandonComplete(); // we have abandoned all forms
      void dataAcquireComplete(const dovah::tes_file_reading::read_results&); // we have loaded new files and forms
      void dataAcquireFailed(const dovah::tes_file_reading::read_results&);   // we tried to load new files, but failed
      //
      void fileLoadWarningReceived(const dovah::detailed_notice&);
      void fileLoadStatisticsAvailable(const file_load_stats&);
      //
      void formModificationImminent(dovah::form_stub*); // emit this before changing a form, so that listeners can update any Use Info they are displaying
      void formModified(dovah::form_stub*); // you should emit this manually when you change a form in a way that other windows/widgets might need to know about, e.g. changing the editor ID
      void formCreated(dovah::form_stub*);
      //
      void formWorkingCopyCommitImminent(dovah::form_stub*); // committing a working copy implies its deletion, by the way
      void formWorkingCopyCommitComplete(dovah::form_stub*);
      void formWorkingCopyDeleteImminent(dovah::form_stub*); // not emitted if the form itself is deleted, data is abandoned, etc.
      void formWorkingCopyDeleteComplete(dovah::form_stub*);
      void questWorkingCopyStagesAltered(dovah::form_stub*);
      void questWorkingCopyAliasesAltered(dovah::form_stub*);
      void packageWorkingCopyPackageDataAltered(dovah::form_stub*);
      //
      void formDeletionImminent(dovah::form_stub*, bool will_be_flagged);
      void formDeletionComplete(dovah::bare_form_id_t, bool will_be_flagged);
      //
      void formRenumbered(dovah::form_stub*, bare_form_id_t oldID, bare_form_id_t newID);
      //
      void formsRenumberedEnMasse();
      //
      void gameSettingValueChanged(const char* name);
      void gameSettingValueChangeFailed(const char* name, dovah::notice_code_t);
      void gameSettingRenumbered(const char* name, bare_form_id_t oldID, bare_form_id_t newID);
      //
      void defaultObjectEntryChanged(uint32_t signature);
      //
      void dataSaveImminent();
      void dataSaveComplete();
      void dataSaveFailed(const dovah::detailed_notice&);
      //
      void editorEncodingChanged(const std::string& prior, const std::string& after);
      //
   public:
      void abandon_data();
      inline bool has_data() const noexcept { return this->loaded; }

      void set_load_order_folder(const std::filesystem::path&);
      void queue_load_order_file(const std::filesystem::path&);
      void unqueue_load_order_file(const std::filesystem::path&);
      void set_load_queued_game(dovah::game);
      void set_queued_active_file(const std::filesystem::path&);
      bool acquire_load_order_data(bool async = false);

      dovah::game get_current_game() const noexcept;

      float assess_load_progress() const noexcept;

      bool for_each_form_edit_dialog(std::function<bool(AbstractFormEditDialog*)>);
      bool for_each_form_uses_dialog(std::function<bool(FormUseInfoDialog*)>);

      std::vector<const dovah::tes_file_reading::file_loader*> get_loaded_files() const noexcept;
      bool loaded_file_is_active(const dovah::tes_file_reading::file_loader&) const noexcept;
      bool active_file_has_name() const noexcept;
      QString get_active_file_name() const noexcept;
      bool has_active_file() const noexcept;
      bool save_active_file(std::filesystem::path name_to_use_if_nameless, const dovah::tes_file_writing::write_config& cfg, dovah::tes_file_writing::write_results& results);
      QString get_active_file_author() const noexcept;
      QString get_active_file_description() const noexcept;
      void set_active_file_author(const QString&) const noexcept;
      void set_active_file_description(const QString&) const noexcept;
      const dovah::tes_file_header* get_active_file_header() const noexcept;

      bool for_each_load_order_filename(std::function<bool(std::filesystem::path, bool is_active_file)> functor) const noexcept;
      bool load_order_has_file(const std::filesystem::path& filename, bool ignore_if_active_file = false) const noexcept;

      uint32_t count_forms_of_type(form_type_t) const noexcept;
      dovah::form_stub* get_form(bare_form_id_t formID) const noexcept;
      dovah::form_stub* get_form(form_type_t, bare_form_id_t formID) const noexcept; // use when you KNOW the form's type
      dovah::form_stub* get_form_of_probable_type(form_type_t, bare_form_id_t formID) const noexcept; // searches (formType) first, then the other types
      dovah::form_stub* get_singleton_form(form_type_t, bool create_if_missing = false) const noexcept; // (create_if_missing) can fail if there are no available form IDs
      bool for_each_form(std::function<bool(dovah::form_stub*)>);
      bool for_each_form_of_type(form_type_t formType, std::function<bool(dovah::form_stub*)>);
      bool for_each_impossible_to_save_form(dovah::game, std::function<bool(dovah::form_stub*)>);

      bool is_form_defined_in_active_file(dovah::form_stub*) const noexcept;

      dovah::form_stub* create_form_of_type(form_type_t);
      dovah::form_creation_request request_form_creation(form_type_t) noexcept;
      dovah::form_duplication_request request_form_duplication() noexcept;
      dovah::form_renumber_request request_form_renumber(dovah::form_stub& stub, bare_form_id_t desiredID) noexcept;

      dovah::form_stub* duplicate_form(dovah::form_stub& original, QWidget* dialog_parent = nullptr); // handles UI, error reporting, etc., for you

      void delete_form(dovah::form_stub& target, QWidget* dialog_parent = nullptr);
      void delete_form(
         dovah::form_stub& target,
         std::function<bool(const dovah::form_deletion_request&)> after_gather, // return false to cancel; this is a good place to report errors or show a confirmation prompt
         std::function<void(const dovah::form_deletion_request&)> after_complete
      );

      bool get_loaded_game_setting(const char* name, dovah::loaded_game_setting& out);
      bool for_each_loaded_game_setting(std::function<bool(const dovah::loaded_game_setting&)>);
      bool edit_game_setting(const char* name, const dovah::game_setting_value&);
      void renumber_game_setting(const char* name, QWidget* dialog_parent); // handles UI, error reporting, etc., for you
      
      void set_default_object(uint32_t signature, dovah::form_stub*);
      void set_default_object(uint32_t signature, bare_form_id_t);

      dovah::bsa_archived_file* lookup_loose_game_asset(const std::filesystem::path&); // path should be relative to, and should not include, the Data directory. caller should delete the returned object, if any
      dovah::bsa_archived_file* lookup_game_asset(const std::filesystem::path&, bool allow_loose_files = true); // path should be relative to, and should not include, the Data directory. caller should delete the returned object, if any
      dovah::compiled_papyrus_script parse_compiled_script(const std::string& scriptname); // can throw exceptions; see definition for dovah::compiled_papyrus_script

      inline const std::string& get_encoding() const noexcept { return this->encoding; }
      void set_encoding(const std::string& name) noexcept; // use the Qt names
      void set_encoding(); // pulls the language name from Skyrim.ini and uses that to decide

      QString convert_localized_string(const dovah::localized_string&) const noexcept;
      void assign_localized_string(dovah::localized_string&, const QString&) const noexcept; // sets the localized_string's contained std::string, i.e. only suitable for when saving something with no STRINGS files

      bool get_game_path(std::filesystem::path& out, dovah::game) const noexcept;
      bool get_game_plugins(std::vector<QString>& out, dovah::game) const noexcept; // plugins.txt

      static QList<QString> list_all_official_plugins(dovah::game, bool mandatory_only = false) noexcept;
};

// IntelliSense doesn't like Q_DECLARE_METATYPE; ignore errors here unless they're compiler errors:
Q_DECLARE_METATYPE(DovahKitCore::file_load_stats)
// needed so that QObject::connect can pass these across threads (by copying them). refer to DovahKitCore's constructor as well.