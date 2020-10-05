#pragma once
#include <filesystem>
#include <unordered_map>
#include <QDialog>
#include <QObject>
#include "../dovah/core.h"
#include "../dovah/files/file_load_order.h"

namespace dovah {
   class  form_stub;
   class  file_write_error;
   struct tes_file_header;
   namespace tes_file_writing {
      struct write_config;
   }
}
namespace DovahKitEditorInternals {
   class load_task;
}

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
      //
      std::unordered_map<bare_form_id_t, QDialog*> extant_form_edit_dialogs;
      std::unordered_map<bare_form_id_t, QDialog*> extant_use_info_dialogs;
      //
      void _configure_load_order();
      //
   signals:
      void dataAbandonImminent(); // we are about to abandon all forms; ditch your pointers or risk memory corruption
      void dataAbandonComplete(); // we have abandoned all forms
      void dataAcquireComplete(); // we have loaded new files and forms
      void dataAcquireFailed(const dovah::file_read_error&);   // we tried to load new files, but failed
      //
      void fileLoadStatisticsAvailable(const file_load_stats&);
      //
      void formModificationImminent(dovah::form_stub*); // emit this before changing a form, so that listeners can update any Use Info they are displaying
      void formModified(dovah::form_stub*); // you should emit this manually when you change a form in a way that other windows/widgets might need to know about, e.g. changing the editor ID
      void formCreated(dovah::form_stub*);
      //
      void formDeletionImminent(dovah::form_stub*, bool will_be_flagged);
      void formDeletionComplete(dovah::bare_form_id_t, bool will_be_flagged);
      //
      void dataSaveImminent();
      void dataSaveComplete();
      void dataSaveFailed(const dovah::file_write_error&);
      //
   public:
      void abandon_data();
      inline bool has_data() const noexcept { return this->loaded; }

      void set_load_order_folder(const std::filesystem::path&);
      void queue_load_order_file(const std::filesystem::path&);
      void unqueue_load_order_file(const std::filesystem::path&);
      void set_queued_active_file(const std::filesystem::path&);
      bool acquire_load_order_data(bool async = false);

      float assess_load_progress() const noexcept;
      const dovah::file_read_error& get_last_read_error() const noexcept;

      bool active_file_has_name() const noexcept;
      QString get_active_file_name() const noexcept;
      bool has_active_file() const noexcept;
      bool save_active_file(std::filesystem::path name_to_use_if_nameless, const dovah::tes_file_writing::write_config* cfg = nullptr);
      QString get_active_file_author() const noexcept;
      QString get_active_file_description() const noexcept;
      void set_active_file_author(const QString&) const noexcept;
      void set_active_file_description(const QString&) const noexcept;
      const dovah::tes_file_header* get_active_file_header() const noexcept;

      bool for_each_load_order_filename(std::function<bool(std::filesystem::path, bool is_active_file)> functor) const noexcept;
      int load_order_index_of_file(const std::filesystem::path& filename);

      const dovah::file_write_error& get_last_write_error() const noexcept;
      const dovah::file_write_warning& get_write_warning() const noexcept;

      uint32_t count_forms_of_type(form_type_t) const noexcept;
      dovah::form_stub* get_form(bare_form_id_t formID) const noexcept;
      dovah::form_stub* get_form(form_type_t, bare_form_id_t formID) const noexcept; // use when you KNOW the form's type
      dovah::form_stub* get_form_of_probable_type(form_type_t, bare_form_id_t formID) const noexcept; // searches (formType) first, then the other types
      bool for_each_form(std::function<bool(dovah::form_stub*)>);
      bool for_each_form_of_type(form_type_t formType, std::function<bool(dovah::form_stub*)>);
      bool form_is_from_active_file(const dovah::form_stub*) const noexcept;
      bool form_is_from_active_file(bare_form_id_t) const noexcept;

      dovah::form_stub* create_form_of_type(form_type_t);
      dovah::form_creation_request request_form_creation(form_type_t) noexcept;
      dovah::form_duplication_request request_form_duplication() noexcept;

      dovah::form_stub* duplicate_form(dovah::form_stub& original, QWidget* dialog_parent = nullptr); // handles UI, error reporting, etc., for you

      void delete_form(dovah::form_stub& target, QWidget* dialog_parent = nullptr);

      bool get_game_path(std::filesystem::path& out) const noexcept;
      bool get_game_plugins(std::vector<QString>& out) const noexcept; // plugins.txt
};

// IntelliSense doesn't like Q_DECLARE_METATYPE; ignore errors here unless they're compiler errors:
Q_DECLARE_METATYPE(DovahKitCore::file_load_stats)
// needed so that QObject::connect can pass these across threads (by copying them). refer to DovahKitCore's constructor as well.