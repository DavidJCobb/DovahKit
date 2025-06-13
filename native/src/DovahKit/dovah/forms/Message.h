#pragma once
#include <array>
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "../data/skills.h"

namespace dovah::loaded_forms {
   class Message : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::message;
         Message(const constructor_params& c) : Form(form_type, c) {};

         struct button {
            components::condition_list conditions; // CTDA[]
            localized_string text = localized_string(localized_string_type::common); // ITXT
         };

         struct flag {
            enum type : uint32_t {
               message_box  = 1 << 0,
               auto_display = 1 << 1,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name        = localized_string(localized_string_type::common);      // FULL
         localized_string description = localized_string(localized_string_type::description); // DESC
         //
         flags_t          flags = 0;        // DNAM
         uint32_t         display_time = 0; // TNAM
         form_reference_t icon; // INAM (-> MICO?)
         form_reference_t owning_quest; // QNAM
         std::vector<button> buttons;

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