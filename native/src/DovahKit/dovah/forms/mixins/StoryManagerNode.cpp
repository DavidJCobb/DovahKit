#include "StoryManagerNode.h"
#include "../_common_cpp.h"

#include "../../data/hardcoded_form_ids.h"
#include "../../notices/form_load_warnings/by_form_type/story_manager_node/orphaned.h"
#include "../../notices/form_load_warnings/by_form_type/story_manager_node/root_is_not_the_root.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::story_manager_node;
   }
}

namespace dovah::loaded_forms::mixins {
   void StoryManagerNode::load(Form& self, tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      if (!intfc.is_winning_record)
         return;

      while (auto& subrecord = record.next_subrecord()) {
         bool stop = false;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'CITC':
               {
                  uint32_t count = 0;
                  if (subrecord.read(count))
                     this->conditions.reserve(count);
               }
               break;
            case 'CTDA':
               this->conditions.read_next(subrecord.get_containing_record(), intfc);
               break;
            case 'PNAM':
               if (auto& form = this->parent; subrecord.read(form)) {
                  this->has_parent = !!form;
                  intfc.warn_if_ref_is_wrong_type(
                     form,
                     std::array{
                        form_type::story_branch_node,
                        form_type::story_event_node
                     },
                     subrecord.signature()
                  );
               } else {
                  this->has_parent = true;
               }
               break;
            case 'SNAM':
               if (auto& form = this->previous_sibling; subrecord.read(form)) {
                  this->has_previous_sibling = !!form;
                  intfc.warn_if_ref_is_wrong_type(
                     form,
                     std::array{
                        form_type::story_branch_node,
                        form_type::story_event_node,
                        form_type::story_quest_node
                     },
                     subrecord.signature()
                  );
               } else {
                  this->has_previous_sibling = true;
               }
               break;
            case 'DNAM':
               subrecord.read(this->flags);
               break;
            case 'XNAM':
               subrecord.read(this->max_concurrent_quests);
               break;
            case 'ENAM':
            case 'MNAM':
            case 'QNAM':
               stop = true;
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
         if (stop)
            break;
      }
      if (!self.stub.is_hardcoded()) {
         if (!this->parent) {
            specific_load_warnings::orphaned notice(self.stub);
            intfc.log_load_warning(notice);
         }
      } else if (self.stub.formID == dovah::hardcoded_form_ids::Root) {
         if (this->parent) {
            specific_load_warnings::root_is_not_the_root notice(self.stub);
            intfc.log_load_warning(notice);
         }
      }
   }
   /*static*/ void StoryManagerNode::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      form_id_t parent;
      form_id_t previous_sibling;

      while (auto& subrecord = record.next_subrecord()) {
         bool stop = false;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'PNAM':
               subrecord.read(parent);
               break;
            case 'SNAM':
               subrecord.read(previous_sibling);
               break;
            case 'ENAM':
            case 'MNAM':
            case 'QNAM':
               stop = true;
               break;
         }
         if (stop)
            break;
      }
      uib.add_outbound_reference(parent);
      uib.add_outbound_reference(previous_sibling);
   }
   void StoryManagerNode::_clone_impl(Form& out_form, StoryManagerNode& out_mixin) const noexcept {
      out_mixin.conditions.clear(out_form);
      out_mixin.conditions.append_all_of(out_form, this->conditions);
      out_mixin.script_data.clone_from(this->script_data, out_form);

      out_mixin.parent.set(out_form, this->parent);
      out_mixin.previous_sibling.set(out_form, this->previous_sibling);
      out_mixin.flags = this->flags;
      out_mixin.max_concurrent_quests = this->max_concurrent_quests;
   }
   void StoryManagerNode::_save_impl(Form& self, tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      record.write_formID_subrecord('PNAM', this->parent);
      record.write_formID_subrecord('SNAM', this->previous_sibling);
      this->has_parent = !!this->parent;
      this->has_previous_sibling = !!this->previous_sibling;
      {
         auto& list = this->conditions;
         {
            auto& subrecord = record.open_next_subrecord('CITC');
            subrecord.write((uint32_t)list.size());
            subrecord.close();
         }
         for (auto& item : list)
            item.save(record, intfc);
      }
      {
         auto& subrecord = record.open_next_subrecord('DNAM');
         subrecord.write(this->flags);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('XNAM');
         subrecord.write(this->max_concurrent_quests);
         subrecord.close();
      }
   }
   void StoryManagerNode::_clear_impl(Form& self) noexcept {
      this->conditions.clear(self);
      this->script_data.clear(self);
      this->parent.set(self, nullptr);
      this->previous_sibling.set(self, nullptr);
      this->flags = 0;
      this->max_concurrent_quests = 1;
   }
   void StoryManagerNode::_sever_outbound_references_impl(Form& self, form_stub& other) noexcept {
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(other, self);
      this->script_data.sever_outbound_references_to(other, self);
      this->parent.clear_if(self, other);
      this->previous_sibling.clear_if(self, other);
   }
}