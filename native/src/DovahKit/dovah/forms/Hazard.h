#pragma once
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Hazard : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::hazard;
         Hazard(const constructor_params& c) : Form(form_type, c) {};

         struct hazard_flag {
            hazard_flag() = delete;
            enum type : uint32_t {
               only_affects_player = 0x0001,
               inherit_duration_from_source_spell = 0x0002,
               align_to_impact_normal = 0x0004,
               inherit_radius_from_source_spell = 0x0008,
               drop_to_ground = 0x0010,
            };
         };

         components::object_bounds bounds; // OBND
         components::model_ts model; // MODL, MODT, MODS
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name; // FULL
         form_reference_t imagespace_modifier; // MNAM
         uint32_t limit    = 0;
         float    radius   = 0;
         float    lifetime = 0;
         float    imagespace_radius = 0;
         float    target_interval   = 0;
         uint32_t hazard_flags = 0;
         form_reference_t spell;
         form_reference_t light;
         form_reference_t impact_data_set;
         form_reference_t sound;

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