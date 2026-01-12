#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/conditions.h"
#include "components/papyrus.h"
#include "../use_info/entry_flags/dialogue_branch.h"

namespace dovah::loaded_forms {
   class DialogueBranch : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::dialogue_branch;
         DialogueBranch(const constructor_params& c) : Form(form_type, c) {};

         struct branch_flag {
            branch_flag() = delete;
            enum type : uint32_t {
               normal    = 0x00, // branch type
               top_level = 0x01, // branch type
               blocking  = 0x02, // branch type
               exclusive = 0x04,
            };
         };
         using branch_flags_t = std::underlying_type_t<branch_flag::type>;

         unique_form_reference_t<use_info::entry_flags::dialogue_branch::parent_quest> owning_quest; // QNAM
         form_reference_t starting_topic; // SNAM
         branch_flags_t   branch_flags = branch_flag::top_level; // DNAM
         uint32_t tnam = 0; // TNAM
         //
         components::object_bounds object_bounds; // OBND. recognized, but probably discarded at run-time.
         components::papyrus_attachment_data script_data; // VMAD

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}