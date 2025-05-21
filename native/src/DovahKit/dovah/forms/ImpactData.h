#pragma once
#include <cstdint>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/decal_data.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "../data/detection_loudness.h"

namespace dovah::loaded_forms {
   class ImpactData : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::impact_data;
         ImpactData(const constructor_params& c) : Form(form_type, c) {};

         enum class effect_orientation : uint32_t {
            surface_normal,
            projectile_vector,
            projectile_reflection,
         };
         enum class impact_result_type : uint8_t {
            default_result,
            destroy,
            bounce,
            impale,
            stick,
         };

      public:
         components::decal_data* decal_data = nullptr; // DODT
         components::model model; // MODL, MODT
         components::papyrus_attachment_data script_data; // VMAD
         //
         float angle_threshold  = 0.0F; // DATA+0x08
         float placement_radius = 0.0F; // DATA+0x0C
         struct {
            bool enabled = false; // DATA+0x14, uint8_t, bit 0
            struct {
               form_reference_t primary;   // DNAM
               form_reference_t secondary; // ENAM
            } texture_sets;
         } decal;
         struct {
            float              duration    = 1.0F; // DATA+0x00
            effect_orientation orientation = effect_orientation::surface_normal; // DATA+0x04
         } effect;
         form_reference_t hazard; // NAM2
         impact_result_type impact_result = impact_result_type::default_result;
         detection_loudness loudness = detection_loudness::normal; // DATA+0x10, uint32_t
         std::array<form_reference_t, 2> sounds; // SNAM, NAM1

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