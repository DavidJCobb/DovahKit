#pragma once
#include <vector>
#include "dovah/callback_sets/data_load.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_collection.h"
#include "dovah/tes/file.h"

#include "dovah/notices/file_parse_warnings/singleton_form_is_redundantly_defined.h"

namespace dovah::load_order_processes::build_form_stubs_from_file {
   inline void set_file_list_for_newest_singleton_stub(
      form_stub_collection& fsc,
      const callback_sets::data_load& callbacks,
      const tes::file& source_file,
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
         if (callbacks.on_warning) {
            auto* pf = prior->get_file_at_index(-1);
            if (pf && pf == &source_file) {
               (callbacks.on_warning)(
                  notices::file_parse_warnings::singleton_form_is_redundantly_defined(
                     source_file.path().filename().string(),
                     newest_record_id,
                     newest_stub,
                     *prior
                  )
               );
            }
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
      newest_stub._replace_source_file_list({}, std::move(stub_files));
   }
}