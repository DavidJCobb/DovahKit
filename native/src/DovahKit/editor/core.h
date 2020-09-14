#pragma once
#include <filesystem>
#include <QObject>
#include "../dovah/core.h"
#include "../dovah/files/file_load_order.h"

namespace dovah {
   class form_stub;
}

class DovahKitCore : public QObject {
   Q_OBJECT
   public:
      using form_type_t    = dovah::form_type_t;
      using bare_form_id_t = dovah::bare_form_id_t;
      static DovahKitCore& get() {
         static DovahKitCore instance;
         return instance;
      }
      ~DovahKitCore();
      //
   protected:
      dovah::file_load_order* load_order = new dovah::file_load_order;
      bool loaded = false;
      //
   signals:
      void dataAbandonImminent(); // we are about to abandon all forms; ditch your pointers or risk memory corruption
      void dataAbandonComplete(); // we have abandoned all forms
      void dataAcquireComplete(); // we have loaded new files and forms
      void dataAcquireFailed(const dovah::file_read_error&);   // we tried to load new files, but failed
      void formModified(dovah::form_stub*); // you should emit this manually when you change a form in a way that other windows/widgets might need to know about, e.g. changing the editor ID
      //
   public:
      void abandon_data();
      inline bool has_data() const noexcept { return this->loaded; }
      void set_load_order_folder(const std::filesystem::path&);
      void queue_load_order_file(const std::filesystem::path&);
      void unqueue_load_order_file(const std::filesystem::path&);
      bool acquire_load_order_data();

      const dovah::file_read_error& get_last_read_error() const noexcept;

      uint32_t count_forms_of_type(form_type_t) const noexcept;
      dovah::form_stub* get_form(bare_form_id_t formID) const noexcept;
      dovah::form_stub* get_form(form_type_t, bare_form_id_t formID) const noexcept; // use when you KNOW the form's type
      dovah::form_stub* get_form_of_probable_type(form_type_t, bare_form_id_t formID) const noexcept; // searches (formType) first, then the other types
      bool for_each_form(std::function<bool(dovah::form_stub*)>);
      bool for_each_form_of_type(form_type_t formType, std::function<bool(dovah::form_stub*)>);
};