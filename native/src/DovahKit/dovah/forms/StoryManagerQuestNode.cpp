#include "StoryManagerQuestNode.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/story_manager_quest_node/expected_quest_subrecord.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::story_manager_quest_node;
   }
}

namespace dovah::loaded_forms {
   void StoryManagerQuestNode::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      mixins::StoryManagerNode::load(*this, record, intfc);

      if (auto& subrecord = record.get_current_subrecord(); subrecord.signature() == 'MNAM') {
         subrecord.read(this->num_quests_to_run);
         record.next_subrecord();
      }
      if (auto& subrecord = record.get_current_subrecord(); subrecord.signature() == 'QNAM') {
         uint32_t quest_count = 0;
         subrecord.read(quest_count);
         record.next_subrecord();
         for (uint32_t i = 0; i < quest_count; ++i) {
            auto& item = this->quests.emplace_back();
            {
               auto& subrecord = record.get_current_subrecord();
               if (auto& subrecord = record.get_current_subrecord(); subrecord.signature() == 'NNAM') {
                  specific_load_warnings::expected_quest_subrecord notice(
                     this->stub,
                     i,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
               }
               if (auto& form = item.form; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::quest, subrecord.signature());
               record.next_subrecord();
            }
            if (auto& subrecord = record.get_current_subrecord(); subrecord.signature() == 'FNAM') {
               subrecord.read(item.flags);
               record.next_subrecord();
            }
            if (auto& subrecord = record.get_current_subrecord(); subrecord.signature() == 'RNAM') {
               subrecord.read(item.raw_hours_until_reset);
               record.next_subrecord();
            }
         }
      }
   }
   /*static*/ void StoryManagerQuestNode::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      mixins::StoryManagerNode::generate_use_info(record, uib);

      if (auto& subrecord = record.get_current_subrecord(); subrecord.signature() == 'MNAM')
         record.next_subrecord();

      if (auto& subrecord = record.get_current_subrecord(); subrecord.signature() == 'QNAM') {
         uint32_t quest_count = 0;
         subrecord.read(quest_count);
         record.next_subrecord();
         for (uint32_t i = 0; i < quest_count; ++i) {
            {
               form_id_t quest;

               auto& subrecord = record.get_current_subrecord();
               subrecord.read(quest);
               record.next_subrecord();

               uib.add_outbound_reference(quest);
            }
            if (auto& subrecord = record.get_current_subrecord(); subrecord.signature() == 'FNAM')
               record.next_subrecord();
            if (auto& subrecord = record.get_current_subrecord(); subrecord.signature() == 'RNAM')
               record.next_subrecord();
         }
      }
   }
   void StoryManagerQuestNode::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (StoryManagerQuestNode*)out;

      mixins::StoryManagerNode::_clone_impl(*copy, *copy);

      copy->num_quests_to_run = this->num_quests_to_run;
      {
         auto& src_list = this->quests;
         auto& dst_list = copy->quests;
         for (auto& item : dst_list)
            item.form.set(*copy, nullptr);
         size_t size = src_list.size();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            dst_list[i].form.set(*copy, src_list[i].form);
            dst_list[i].flags = src_list[i].flags;
            dst_list[i].raw_hours_until_reset = src_list[i].raw_hours_until_reset;
         }
      }
   }
   void StoryManagerQuestNode::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      mixins::StoryManagerNode::_save_impl(*this, record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('MNAM');
         subrecord.write(this->num_quests_to_run);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('QNAM');
         subrecord.write((uint32_t)this->quests.size());
         subrecord.close();
         for (auto& item : this->quests) {
            record.write_formID_subrecord('NNAM', item.form);
            if (item.flags) {
               auto& subrecord = record.open_next_subrecord('FNAM');
               subrecord.write(item.flags);
               subrecord.close();
            }
            if (item.raw_hours_until_reset) {
               auto& subrecord = record.open_next_subrecord('RNAM');
               subrecord.write(item.raw_hours_until_reset);
               subrecord.close();
            }
         }
      }
   }
   void StoryManagerQuestNode::_clear_impl() noexcept {
      mixins::StoryManagerNode::_clear_impl(*this);

      this->num_quests_to_run = 1;
      for (auto& item : this->quests)
         item.form.set(*this, nullptr);
      this->quests.clear();
   }
   void StoryManagerQuestNode::_sever_outbound_references_impl(form_stub& other) noexcept {
      mixins::StoryManagerNode::_sever_outbound_references_impl(*this, other);

      auto& list    = this->quests;
      bool  changed = false;
      for (auto& item : list) {
         item.form.clear_if(*this, other);
         if (!item.form)
            changed = true;
      }
      if (changed)
         std::erase_if(list, [](auto& item) { return !item.form; });
   }
}