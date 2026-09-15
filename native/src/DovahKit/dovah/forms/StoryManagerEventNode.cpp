#include "StoryManagerEventNode.h"
#include "_common_cpp.h"

#include "../data/story_manager.h"
#include "../notices/form_load_warnings/by_form_type/story_manager_event_node/unrecognized_event.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::story_manager_event_node;
   }
}

namespace dovah::loaded_forms {
   void StoryManagerEventNode::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      mixins::StoryManagerNode::load(*this, record, intfc);

      {
         auto& subrecord = record.get_current_subrecord();
         if (subrecord.signature() == 'ENAM') {
            subrecord.read_signature(this->event);
            record.next_subrecord();
         }
      }

      if (this->event) {
         bool found = false;
         for (const auto i : all_story_event_codes) {
            if (i == this->event) {
               found = true;
               break;
            }
         }
         if (!found) {
            specific_load_warnings::unrecognized_event notice(
               this->stub,
               this->event
            );
            intfc.log_load_warning(notice);
         }
      }
   }
   /*static*/ void StoryManagerEventNode::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      mixins::StoryManagerNode::generate_use_info(record, uib);

      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == 'ENAM') {
         record.next_subrecord();
      }
   }
   void StoryManagerEventNode::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (StoryManagerEventNode*)out;

      mixins::StoryManagerNode::_clone_impl(*copy, *copy);

      copy->event = this->event;
   }
   void StoryManagerEventNode::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      mixins::StoryManagerNode::_save_impl(*this, record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('ENAM');
         subrecord.write_signature(this->event);
         subrecord.close();
      }
   }
   void StoryManagerEventNode::_clear_impl() noexcept {
      mixins::StoryManagerNode::_clear_impl(*this);
      this->event = 0;
   }
   void StoryManagerEventNode::_sever_outbound_references_impl(form_stub& other) noexcept {
      mixins::StoryManagerNode::_sever_outbound_references_impl(*this, other);
   }
}