#pragma once

namespace dovah {
   namespace tes_file_writing {
      class file_writer;
   }
   struct detailed_notice;
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

         // TIP: This function only logs a warning if it has a warning code.
         void log_save_warning(detailed_notice&);

         constexpr const form_stub* get_previous_child() const noexcept { return this->previous_child; }

         // The file writer can only retain one save error.
         void set_save_error(const detailed_notice&);
            
      protected:
         form_save(file_load_order& o, tes_file_writing::file_writer& w) : owner(o), writer(w) {}
   };
}