#pragma once
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class VolumetricLighting : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::volumetric_lighting;
         VolumetricLighting(const constructor_params& c) : Form(form_type, c) {};

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         float intensity = 2; // CNAM // init'd to INI::Display::fVolumetricLightingIntensity
         struct {
            float contribution = 0; // DNAM // init'd to INI::Display::fVolumetricLightingCustomColorContribution
            struct {
               float r = 1; // ENAM
               float g = 1; // FNAM
               float b = 1; // GNAM
            } color;
         } custom_color;
         struct {
            float contribution =   0.3; // HNAM // init'd to INI::Display::fVolumetricLightingDensityContribution
            float size         = 300;   // INAM // init'd to INI::Display::fVolumetricLightingDensityScale
            struct {
               float falling =  0.3; // KNAM // init'd to INI::Display::fVolumetricLightingWindFallingSpeed
               float wind    = 15;   // JNAM // init'd to INI::Display::fVolumetricLightingWindSpeedScale
            } speeds;
         } density;
         struct {
            float contribution = 0.83; // LNAM // init'd to INI::Display::fVolumetricLightingPhaseContribution
            float scattering   = 0.85; // MNAM // init'd to INI::Display::fVolumetricLightingPhaseScattering
         } phase_function;
         struct {
            float range_factor = 40; // NNAM // max 1.0 // init'd to INI::Display::fVolumetricLightingRangeFactor
         } sampling_repartition;

      public:
         virtual void setup(const file_load_order&) noexcept override;
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}