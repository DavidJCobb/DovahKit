#pragma once
#include <filesystem>
#include <unordered_map>
#include <QDialog>
#include <QObject>
#include "../dovah/core.h"
#include "../dovah/files/file_load_order.h"

namespace dovah {
   class form_stub;
}
namespace DovahKitEditorInternals {
   class load_task;
}

class DovahKitCore : public QObject {
   Q_OBJECT
   friend class DovahKitEditorInternals::load_task;
   friend void open_window_for_form(dovah::form_stub*, QWidget* parent);
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
      dovah::file_load_order* load_order = new dovah::file_load_order;
      bool    loaded  = false;
      bool    loading = false;
      QThread* async_loader = nullptr;
      //
      std::unordered_map<bare_form_id_t, QDialog*> extant_form_edit_dialogs;
      //
   signals:
      void dataAbandonImminent(); // we are about to abandon all forms; ditch your pointers or risk memory corruption
      void dataAbandonComplete(); // we have abandoned all forms
      void dataAcquireComplete(); // we have loaded new files and forms
      void dataAcquireFailed(const dovah::file_read_error&);   // we tried to load new files, but failed
      //
      void fileLoadStatisticsAvailable(const file_load_stats&);
      //
      void formModified(dovah::form_stub*); // you should emit this manually when you change a form in a way that other windows/widgets might need to know about, e.g. changing the editor ID
      //
   public:
      void abandon_data();
      inline bool has_data() const noexcept { return this->loaded; }
      void set_load_order_folder(const std::filesystem::path&);
      void queue_load_order_file(const std::filesystem::path&);
      void unqueue_load_order_file(const std::filesystem::path&);
      void set_queued_active_file(const std::filesystem::path&);
      bool acquire_load_order_data(bool async = false);

      const dovah::file_read_error& get_last_read_error() const noexcept;

      uint32_t count_forms_of_type(form_type_t) const noexcept;
      dovah::form_stub* get_form(bare_form_id_t formID) const noexcept;
      dovah::form_stub* get_form(form_type_t, bare_form_id_t formID) const noexcept; // use when you KNOW the form's type
      dovah::form_stub* get_form_of_probable_type(form_type_t, bare_form_id_t formID) const noexcept; // searches (formType) first, then the other types
      bool for_each_form(std::function<bool(dovah::form_stub*)>);
      bool for_each_form_of_type(form_type_t formType, std::function<bool(dovah::form_stub*)>);
      bool form_is_from_active_file(const dovah::form_stub*) const noexcept;
      bool form_is_from_active_file(bare_form_id_t) const noexcept;

      bool get_game_path(std::filesystem::path& out) const noexcept;
      bool get_game_plugins(std::vector<QString>& out) const noexcept; // plugins.txt
};

// IntelliSense doesn't like Q_DECLARE_METATYPE; ignore errors here unless they're compiler errors:
Q_DECLARE_METATYPE(DovahKitCore::file_load_stats)
// needed so that QObject::connect can pass these across threads (by copying them). refer to DovahKitCore's constructor as well.