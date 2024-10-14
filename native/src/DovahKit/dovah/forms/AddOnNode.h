#pragma once
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class AddOnNode : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::addon_node;
         AddOnNode(const constructor_params& c) : Form(form_type, c) {};

         components::object_bounds bounds; // OBND
         components::model_ts      model;  // MODL, MODT, MODS
         components::papyrus_attachment_data script_data; // VMAD
         //
         int32_t          unique_id = -1; // DATA
         form_reference_t sound;          // SNAM
         uint16_t         master_particle_system_cap = 0; // DNAM+00 // CK forces this to zero if DNAM+02 bit (1 << 0) is not set
         struct {
            bool is_valid_master_particle_system = false; // DNAM+02 bit (1 << 0)
            bool always_loaded = false; // DNAM+02 bit (1 << 1)
         } addon_flags;

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