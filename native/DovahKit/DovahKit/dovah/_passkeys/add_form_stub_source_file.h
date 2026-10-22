#pragma once
namespace dovah {
   namespace tes {
      class file;
   }
   class form_stub_collection;

   namespace impl::_form_stub_collection {
      extern void create_hardcoded_form_stubs(form_stub_collection&, tes::file& hardcoded_forms_file);
      extern void create_none_stubs(form_stub_collection&, tes::file& none_stubs_file);
   }
}

namespace dovah::_passkeys {
   class add_form_stub_source_file {
      friend void impl::_form_stub_collection::create_hardcoded_form_stubs(form_stub_collection&, tes::file&);
      friend void impl::_form_stub_collection::create_none_stubs(form_stub_collection&, tes::file&);
      private:
         constexpr add_form_stub_source_file() {}
   };
}