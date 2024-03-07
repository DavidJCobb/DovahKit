#pragma once
#include <cstdint>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class FormList : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::formlist;
         FormList(const constructor_params& c) : Form(form_type, c) {};

         components::papyrus_attachment_data script_data;
         components::object_bounds bounds;
         //
         std::vector<form_reference_t> contents;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}