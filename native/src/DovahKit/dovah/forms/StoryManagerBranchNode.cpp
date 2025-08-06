#include "StoryManagerBranchNode.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void StoryManagerBranchNode::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      mixins::StoryManagerNode::load(*this, record, intfc);
   }
   /*static*/ void StoryManagerBranchNode::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      mixins::StoryManagerNode::generate_use_info(record, uib);
   }
   void StoryManagerBranchNode::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (StoryManagerBranchNode*)out;

      mixins::StoryManagerNode::_clone_impl(*copy, *copy);
   }
   void StoryManagerBranchNode::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      mixins::StoryManagerNode::_save_impl(*this, record, intfc);
   }
   void StoryManagerBranchNode::_clear_impl() noexcept {
      mixins::StoryManagerNode::_clear_impl(*this);
   }
   void StoryManagerBranchNode::_sever_outbound_references_impl(form_stub& other) noexcept {
      mixins::StoryManagerNode::_sever_outbound_references_impl(*this, other);
   }
}