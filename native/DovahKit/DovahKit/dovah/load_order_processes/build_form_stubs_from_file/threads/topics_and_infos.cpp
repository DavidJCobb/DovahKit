#include "./topics_and_infos.h"
#include "dovah/active_load_order.h"
#include "dovah/callback_sets/data_load.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_collection.h"
#include "dovah/form_type.h"
#include "dovah/tes/file.h"
#include "dovah/tes/reading/file_view.h"
#include "../make_stub_from_record.h"
#include "../process.h"
#include "./impl/fail_if_form_fatally_misplaced.h"
#include "./impl/get_parent_stub_for_group.h"

#include "dovah/exceptions/file_parse_errors/unexpected_nested_group.h"
#include "dovah/notices/file_parse_warnings/record_found_in_wrong_group.h"

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   void topics_and_infos::execute_on_thread() {
      auto& alo         = this->_config.active_load_order;
      auto& callbacks   = this->_config.callbacks;
      auto& source_file = this->_config.source_file;

      tes::file_view view;
      view.set_target_file(&source_file);

      const auto& topic_group      = view.groups[0];
      const auto& topic_info_group = view.groups[1];

      const size_t size = this->queue.size();
      this->_progress.maximum = (uint32_t)size;
      for (uint32_t i = 0; i < size; i++) {
         this->_progress.current = i;
         auto& desired = this->queue[i];
         view.set_position(desired.pos);
         assert(view.next_record_or_group() == tes::file_view::chunk_type::group); // TODO: error instead

         form_stub* topic_stub = nullptr;

         tes::file_view::chunk_type ot;
         while (ot = view.next_record_or_group(), ot != tes::file_view::chunk_type::none) {
            if (!topic_group.signature) {
               //
               // We've reached the end of the topic group.
               //
               break;
            }
            if (ot == tes::file_view::chunk_type::group) {
               if (view.groups[2].signature) {
                  //
                  // We were already in the topic-info group, and we just opened a nested group inside 
                  // of that. No such group should be present.
                  //
                  throw exceptions::file_parse_errors::unexpected_nested_group(
                     source_file,
                     topic_info_group.position,
                     topic_info_group,
                     view.groups[2].position,
                     view.groups[2]
                  );
               }
               topic_stub = &impl::get_parent_stub_for_group<form_type::topic>(this->_config, topic_info_group.position, topic_info_group);
            }
            if (ot == tes::file_view::chunk_type::record) {
               auto& record = view.subviews.record;
               auto* stub   = make_stub_from_record(source_file, record.file_offset_of_header(), record.header());
               if (topic_info_group.signature) {
                  //
                  // Loaded [what should be] a topic info.
                  //
                  assert(!!topic_stub);
                  if (stub->form_type == form_type::topic_info) {
                     stub->_set_parent_form_one_way({}, topic_stub);
                  } else {
                     impl::fail_if_form_fatally_misplaced(this->_config, record, *stub);
                     if (callbacks.on_warning) {
                        (callbacks.on_warning)(
                           notices::file_parse_warnings::record_found_in_wrong_group(
                              source_file.path().filename().string(),
                              topic_info_group.position,
                              topic_info_group,
                              topic_stub,
                              record.file_offset_of_header(),
                              record.header(),
                              stub
                           )
                        );
                     }
                  }
               } else {
                  //
                  // Loaded [what should be] a topic.
                  //
                  if (stub->form_type != form_type::topic) {
                     impl::fail_if_form_fatally_misplaced(this->_config, record, *stub);
                     if (callbacks.on_warning) {
                        (callbacks.on_warning)(
                           notices::file_parse_warnings::record_found_in_wrong_group(
                              source_file.path().filename().string(),
                              topic_group.position,
                              topic_group,
                              nullptr,
                              record.file_offset_of_header(),
                              record.header(),
                              stub
                           )
                        );
                     }
                  }
               }
               this->_finish_stub(std::move(stub), record);
            }
         }
      }
   }
}