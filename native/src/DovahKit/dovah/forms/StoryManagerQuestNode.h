#pragma once
#include <cstdint>
#include <vector>
#include "./Form.h"
#include "./_common.h"
#include "../data/story_manager.h"
#include "./mixins/StoryManagerNode.h"

namespace dovah::loaded_forms {
   class StoryManagerQuestNode : public Form, public mixins::StoryManagerNode {
      public:
         static constexpr const enum form_type form_type = form_type::story_quest_node;
         StoryManagerQuestNode(const constructor_params& c) : Form(form_type, c) {};

         struct quest_entry {
            public:
               struct flag {
                  enum type : uint32_t {
                     reset_after_24_hours = 1 << 0,
                  };
               };
               using flags_t = std::underlying_type_t<flag::type>;

            public:
               form_reference_t form; // NNAM -> QUST
               flags_t          flags = 0; // FNAM
               float            hours_until_reset = 0; // RNAM
         };

      public:
         uint32_t num_quests_to_run = 1; // MNAM
         std::vector<quest_entry> quests;

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}