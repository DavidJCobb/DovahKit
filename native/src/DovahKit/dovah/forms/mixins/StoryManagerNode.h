#pragma once
#include <cstdint>
#include "../Form.h"
#include "../_common.h"
#include "../components/conditions.h"
#include "../components/papyrus.h"

namespace dovah::loaded_forms::mixins {
   class StoryManagerNode {
      public:
         struct flag {
            enum type : uint32_t {
               random = 1 << 0,
               warn_if_no_child_quest_started = 1 << 1,

               // quest nodes only
               do_all_before_repeating = 1 << 16,
               shares_event            = 1 << 17,
               has_num_quests_to_run   = 1 << 18, // DovahKit uses this flag only during load, and updates it during save
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

      public:
         components::condition_list          conditions;  // CITC+CTDA[]
         components::papyrus_attachment_data script_data; // VMAD
         //
         form_reference_t parent;           // PNAM -> BGSStoryManagerBranchNode
         form_reference_t previous_sibling; // SNAM -> BGSStoryManagerNodeBase
         bool has_parent           = false; // if PNAM != NONE, even if it was a missing form ID
         bool has_previous_sibling = false; // if SNAM != NONE, even if it was a missing form ID
         flags_t  flags = 0; // DNAM
         uint32_t max_concurrent_quests = 0; // XNAM

      public:
         void load(Form& self, tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         void _clone_impl(Form& out_form, StoryManagerNode& out_mixin) const noexcept;
         void _save_impl(Form& self, tes_record_writer& record, load_order_interfaces::form_save& intfc);
         void _clear_impl(Form& self) noexcept;
         void _sever_outbound_references_impl(Form& self, form_stub& other) noexcept;
   };
}