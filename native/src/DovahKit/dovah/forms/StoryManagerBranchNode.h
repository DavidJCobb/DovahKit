#pragma once
#include <cstdint>
#include "./Form.h"
#include "./_common.h"
#include "./mixins/StoryManagerNode.h"

namespace dovah::loaded_forms {
   class StoryManagerBranchNode : public Form, public mixins::StoryManagerNode {
      public:
         static constexpr const enum form_type form_type = form_type::story_branch_node;
         StoryManagerBranchNode(const constructor_params& c) : Form(form_type, c) {};

      public:
         // No subclass fields.

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