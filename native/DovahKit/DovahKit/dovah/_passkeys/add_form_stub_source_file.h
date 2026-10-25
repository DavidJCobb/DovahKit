#pragma once
#include "dovah/types/bare_form_id.h"
namespace dovah {
   namespace callback_sets {
      struct data_load;
   }
   namespace tes {
      class  file;
      struct record_header;
   }
   class form_stub;
   class form_stub_collection;

   namespace load_order_processes::build_form_stubs_from_file {
      class process;
      extern form_stub* make_stub_from_record(const tes::file&, size_t, const tes::record_header&);
      extern void set_file_list_for_newest_singleton_stub(
         form_stub_collection&,
         const callback_sets::data_load&,
         const tes::file&,
         form_stub&,
         bare_form_id
      );
   }
   namespace impl::_form_stub_collection {
      extern void create_hardcoded_form_stubs(form_stub_collection&, tes::file&);
      extern void create_none_stubs(form_stub_collection&, tes::file&);
   }
}

namespace dovah::_passkeys {
   class add_form_stub_source_file {
      friend load_order_processes::build_form_stubs_from_file::process;
      friend void       impl::_form_stub_collection::create_hardcoded_form_stubs(form_stub_collection&, tes::file&);
      friend void       impl::_form_stub_collection::create_none_stubs(form_stub_collection&, tes::file&);
      friend void       load_order_processes::build_form_stubs_from_file::set_file_list_for_newest_singleton_stub(
         form_stub_collection&,
         const callback_sets::data_load&,
         const tes::file&,
         form_stub&,
         bare_form_id
      );
      friend form_stub* load_order_processes::build_form_stubs_from_file::make_stub_from_record(const tes::file&, size_t, const tes::record_header&);
      private:
         constexpr add_form_stub_source_file() {}
   };
}