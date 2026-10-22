#pragma once
namespace dovah {
   namespace tes {
      class file;
      class record_view;
   }
   class  form_stub;
   struct form_stub_addenda;

   namespace load_order_processes::build_form_stubs_from_file {
      extern void extract_high_value_subrecords_for_stub(tes::file&, form_stub&, tes::record_view&);
   }
}

namespace dovah::form_stub_addendum_types::passkeys {
   class reorder_children_after_load {
      //friend dovah::form_deletion_request;
      friend dovah::form_stub;
      friend dovah::form_stub_addenda;
      friend void load_order_processes::build_form_stubs_from_file::extract_high_value_subrecords_for_stub(tes::file&, form_stub&, tes::record_view&);
      private:
         constexpr reorder_children_after_load() {}
   };
}