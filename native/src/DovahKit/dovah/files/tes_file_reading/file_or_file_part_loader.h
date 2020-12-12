#pragma once
#include "basic_reader.h"
#include "../file_load_order.h"

namespace dovah::tes_file_reading {
   class file_loader;

   class file_or_file_part_loader : public basic_reader {
      using lo_interface_t = load_order_interfaces::file_load;
      private:
         lo_interface_t load_interface;
         //
      protected:
         file_or_file_part_loader(lo_interface_t&);
         //
         virtual file_loader& get_file_loader() const noexcept = 0;
         //
      protected:
         object_type next_record_or_group(); // only called during the initial file read
         bool        next_subrecord(); // called after the initial file read, when loading a form_stub's full content
         //
      protected:
         void log_load_warning(detailed_notice&);
         void log_load_error(detailed_notice&);
         //
         form_stub* make_stub_for_record();
         bool commit_stub(form_stub&);
         void extract_high_value_subrecords_for_stub(form_stub&);
         //
      public:
         file_load_order& get_load_order() const noexcept;
   };
}