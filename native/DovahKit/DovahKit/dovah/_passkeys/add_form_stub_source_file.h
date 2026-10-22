#pragma once
namespace dovah {
   namespace tes {
      class  file;
      struct record_header;
   }
   class form_stub;
   class form_stub_collection;

   namespace load_order_processes::build_form_stubs_from_file {
      inline form_stub* make_stub_from_record(const tes::file&, size_t, const tes::record_header&);
   }
   namespace impl::_form_stub_collection {
      extern void create_hardcoded_form_stubs(form_stub_collection&, tes::file& hardcoded_forms_file);
      extern void create_none_stubs(form_stub_collection&, tes::file& none_stubs_file);
   }
}

namespace dovah::_passkeys {
   class add_form_stub_source_file {
      friend void       impl::_form_stub_collection::create_hardcoded_form_stubs(form_stub_collection&, tes::file&);
      friend void       impl::_form_stub_collection::create_none_stubs(form_stub_collection&, tes::file&);
      friend form_stub* load_order_processes::build_form_stubs_from_file::make_stub_from_record(const tes::file&, size_t, const tes::record_header&);;
      private:
         constexpr add_form_stub_source_file() {}
   };
}