#pragma once
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/container.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class ConstructibleObject : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::constructible_object;
         ConstructibleObject(const constructor_params& c) : Form(form_type, c) {};

         components::condition_list conditions; // CTDA and friends
         components::container_data inventory;
         components::papyrus_attachment_data script_data;
         //
         struct {
            uint16_t         count = 1; // NAM1
            form_reference_t form;      // CNAM
         } result;
         form_reference_t workbench_keyword; // BNAM -> KYWD

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}