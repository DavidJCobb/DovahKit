#pragma once
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Global : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::global;
         Global(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               constant = 0x00000040,
            };
         };

         enum class value_type : uint8_t {
            int16   = 's',
            int32   = 'l',
            float32 = 'f',
         };

         components::papyrus_attachment_data script_data; // VMAD
         //
         enum value_type value_type = value_type::float32;
         float           value      = 0.0F;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}