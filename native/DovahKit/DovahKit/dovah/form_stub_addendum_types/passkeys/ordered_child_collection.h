#pragma once
namespace dovah {
   namespace tes {
      class file;
      class record_view;
   }
   class form_stub;
   class form_stub_addenda;

   namespace load_order_processes::build_form_stubs_from_file {
      extern void extract_high_value_subrecords_for_stub(const tes::file&, form_stub&, tes::record_view&);
   }
}

namespace dovah::form_stub_addendum_types::passkeys {
   class alter_ordered_children_during_load {
      friend void load_order_processes::build_form_stubs_from_file::extract_high_value_subrecords_for_stub(const tes::file&, form_stub&, tes::record_view&);
      private:
         constexpr alter_ordered_children_during_load() {}
   };

   class reorder_children_after_load {
      //friend dovah::form_deletion_request;
      friend dovah::form_stub;
      friend dovah::form_stub_addenda;
      friend void load_order_processes::build_form_stubs_from_file::extract_high_value_subrecords_for_stub(const tes::file&, form_stub&, tes::record_view&);
      private:
         constexpr reorder_children_after_load() {}
   };
}