#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Eyes : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::eyes;
         Eyes(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               non_playable = 0x00000004,
            };
         };

         struct flag {
            enum type : uint8_t {
               playable   = 1 << 0,
               not_male   = 1 << 1,
               not_female = 1 << 2,
            };
         };

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name = localized_string(localized_string_type::common); // FULL
         //
         uint8_t     flags = 0; // DATA
         std::string texture;   // ICON

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