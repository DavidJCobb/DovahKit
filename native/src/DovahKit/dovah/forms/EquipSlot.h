#pragma once
#include <cstdint>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class EquipSlot : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::equip_slot;
         EquipSlot(const constructor_params& c) : Form(form_type, c) {};
         
         struct local_flag {
            local_flag() = delete;
            enum type : uint32_t {
               use_all_parents = 1,
            };
         };
         using local_flags_t = std::underlying_type_t<local_flag::type>;

         components::papyrus_attachment_data script_data;
         //
         std::vector<form_reference_t> parent_slots;
         local_flags_t local_flags = 0;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}