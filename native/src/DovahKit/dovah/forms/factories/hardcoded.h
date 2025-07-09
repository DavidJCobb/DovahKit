#pragma once

namespace dovah {
   class file_load_order;
   class form_stub;
   class form_stub_use_info_builder;
   namespace loaded_forms {
      class Form;
   }

   extern void add_hardcoded_forms_to_load_order(file_load_order&); // create form_stubs and loaded form data for hardcoded forms, and add them to the load order. call this before reading any files.
   extern void build_hardcoded_form_outbound_refs(form_stub_use_info_builder&); // create outbound use info for hardcoded forms that have not been overridden.

   extern loaded_forms::Form* instantiate_hardcoded_form(form_stub&);
}