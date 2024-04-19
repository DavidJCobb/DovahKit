#pragma once
#include "basic_reader.h"
#include "../../load_order_interfaces/file_load.h"
#include "../file_load_order.h"

namespace dovah {
   namespace tes_file_reading {
      class file_loader;
   }
}

namespace dovah::tes_file_reading {
   class file_or_file_part_loader : protected basic_reader {
      //
      // Common base class for (file_loader) and friends. Manages critical communication with 
      // the (file_load_order).
      //
      using lo_interface_t = load_order_interfaces::file_load;
      protected:
         file_or_file_part_loader(file_loader& owner); // yes, i know this is ugly
         file_or_file_part_loader(file_loader& self, lo_interface_t&); // needed for the (file_loader) constructor itself
         
         constexpr file_loader& get_file_loader() const noexcept { return *this->loader; }
         
         lo_interface_t load_interface;
         
      protected:
         object_type next_record_or_group(); // only called during the initial file read
         bool        next_subrecord(); // called after the initial file read, when loading a form_stub's full content
         
      protected:
         void log_load_warning(detailed_notice&);
         void log_load_error(detailed_notice&);
         
         form_stub* make_stub_for_record();

         bool set_stub_parent(form_stub*, bare_form_id_t parentID);

         // Send the stub to the (file_load_order). If the stub is invalid, it will be deleted. If the stub is an 
         // override, then it will be deleted and the passed-in pointer will be made to refer to the overridden 
         // stub. Never returns (true) if the stub is deleted.
         bool commit_stub(form_stub*&);

         void extract_high_value_subrecords_for_stub(form_stub&);
         
      public:
         file_load_order& get_load_order() const noexcept;
         
         #pragma region Grant access to specific (basic_reader) fields
         using basic_reader::load_record_at;
         using basic_reader::get_current_group;
         using basic_reader::get_current_record;
         using basic_reader::get_current_subrecord;
         #pragma endregion
   };
}