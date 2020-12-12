#pragma once
#include "basic_reader.h"
#include "../file_load_order.h"

namespace dovah::tes_file_reading {
   class file_loader;

   class file_or_file_part_loader : protected basic_reader {
      using lo_interface_t = load_order_interfaces::file_load;
      protected:
         file_or_file_part_loader(lo_interface_t&);
         //
         virtual file_loader& get_file_loader() const noexcept = 0;
         //
         lo_interface_t load_interface;
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
         //
         #pragma region Grant access to specific (basic_reader) fields
         using basic_reader::load_record_at;
         inline group& get_current_group() { return *(group*)&this->get_current_group(); }
         inline record& get_current_record() { return *(record*)&this->_record; }
         inline subrecord& get_current_subrecord() { return *(subrecord*)&this->_subrecord; }
         #pragma endregion
   };
}