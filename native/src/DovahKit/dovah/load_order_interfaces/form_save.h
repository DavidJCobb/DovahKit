#pragma once

namespace dovah {
   namespace notices {
      class base_form_save_error;
      class base_form_save_warning;
   }
   namespace tes_file_writing {
      class file_writer;
   }
   class file_load_order;
   class form_stub;
}

namespace dovah::load_order_interfaces {
   class form_save {
      friend class tes_file_writing::file_writer;
      protected:
         form_stub* previous_child = nullptr;
         tes_file_writing::file_writer& writer;
      public:
         file_load_order& owner;
         form_stub* target_stub = nullptr;

         constexpr const form_stub* get_previous_child() const noexcept { return this->previous_child; }

         void log_save_warning(notices::base_form_save_warning&);
         [[noreturn]] void throw_save_error(const notices::base_form_save_error&);
            
      protected:
         form_save(file_load_order& o, tes_file_writing::file_writer& w) : owner(o), writer(w) {}
   };
}