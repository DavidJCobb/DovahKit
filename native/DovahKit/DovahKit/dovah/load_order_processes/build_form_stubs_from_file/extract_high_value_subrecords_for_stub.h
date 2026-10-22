#pragma once
namespace dovah {
   namespace tes {
      class file;
      class record_view;
   }
   class form_stub;
}

namespace dovah::load_order_processes::build_form_stubs_from_file {
   extern void extract_high_value_subrecords_for_stub(const tes::file&, form_stub&, tes::record_view&);
}