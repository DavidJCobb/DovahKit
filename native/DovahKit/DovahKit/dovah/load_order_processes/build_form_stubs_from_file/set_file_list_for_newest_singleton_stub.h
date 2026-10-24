#pragma once
#include <vector>
#include "dovah/callback_sets/data_load.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_collection.h"
#include "dovah/tes/file.h"

namespace dovah::load_order_processes::build_form_stubs_from_file {
   inline void set_file_list_for_newest_singleton_stub(
      form_stub_collection& fsc,
      const callback_sets::data_load& callbacks,
      form_stub&   newest_stub,
      bare_form_id newest_record_id
   ) {
      //
      // There are a limited number of form types in Bethesda RPGs that are handled specially: the 
      // game will only ever create one instance of the form in question, and all subsequent records 
      // of that type will then load data for the preexisting instance, even if their form IDs differ. 
      // 
      // We need to replicate this behavior, while also still being aware that each given form ID was 
      // occupied by a definition. In other words, we need to have it so that multiple form stubs can 
      // contribute to "the same" form. The easiest way to manage that is this: when inserting a DOBJ 
      // form stub, grab the last inserted DOBJ form stub and copy its source file information into 
      // the new form stub. (Why only the last inserted DOBJ stub? Because it, in turn, will have the 
      // information from its own predecessor, and so on.)
      //
      auto* prior = fsc.get_canonical_instance_of_singleton_form(newest_stub.form_type);
      if (!prior || prior == &newest_stub)
         return;
      std::vector<form_stub::file_data> stub_files;
      if (prior) {
         uint16_t count = prior->source_file_count();
         for (uint16_t i = 0; i < count; ++i) {
            auto* data = prior->get_source_file_info(i);
            if (!data || !data->pointer)
               continue;
            stub_files.push_back(*data);
         }
         //
         // Also, let's log a load warning if this singleton form is redundantly defined within the 
         // current file.
         //
         auto* pf = prior->get_source_file_info();
         auto* sf = newest_stub.get_source_file_info();
         if (pf && sf && pf->pointer == sf->pointer) {
            notices::file_load_warnings::singleton_form_is_redundantly_defined notice;
            notice.source_file = sf->pointer->path().filename().string();
            notice.previous_form_id = prior->form_id;
            notice.record = {
               .local_id  = newest_record_id,
               .global_id = newest_stub.form_id,
               .form_type = newest_stub.form_type,
            };
            this->_log_warning(notice);
         }
      }
      //
      // And of course, the new stub already knows its file information, so let's grab that, too.
      //
      uint16_t count = newest_stub.source_file_count();
      for (uint16_t i = 0; i < count; ++i) {
         auto* data = newest_stub.get_source_file_info(i);
         if (!data || !data->pointer)
            continue;
         stub_files.push_back(*data);
      }
      newest_stub._replace_source_file_list(stub_files);
   }
}