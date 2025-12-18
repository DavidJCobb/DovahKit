#pragma once
#include <array>
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Imagespace : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::imagespace;
         Imagespace(const constructor_params& c) : Form(form_type, c) {};

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         struct {
            float eye_adapt_speed = 3;
            struct {
               float blur_radius       = 7;
               float threshold         = 0.6;
               float scale             = 0.5;
               float receive_threshold = 0.15;
            } bloom;
            float white              = 0.15;
            float sunlight_scale     = 1.8;
            float sky_scale          = 1.5;
            float eye_adapt_strength = 3.5;
         } hdr;
         struct {
            float saturation = 0.9;
            float brightness = 1.5;
            float contrast   = 1.1;
         } cinematic;
         struct {
            float amount = 0;
            float r = 0;
            float g = 0;
            float b = 0;
         } tint;
         struct {
            float strength = 0;
            float distance = 0;
            float range    = 0;
            unsigned int radius : 3 = 2;
            bool  no_sky   = false;
         } depth_of_field;

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