#pragma once
#include <cstdint>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Sound : public Form { // TESSound (SOUN)
      public:
         static constexpr const enum form_type form_type = form_type::sound;
         Sound(const constructor_params& c) : Form(form_type, c) {};

         components::object_bounds bounds;
         components::papyrus_attachment_data script_data;
         //
         form_reference_t descriptor; // SDSC // value is SNDR

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