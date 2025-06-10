#pragma once
#include <cstdint>
#include <optional>
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
         
         struct legacy_data {
            std::string path; // FNAM
            // SNDD:
            struct {
               uint8_t  min = 0; // serialized value; real value is this times 5
               uint8_t  max = 0; // serialized value; real value is this times 100
            } attenuation_distance;
            int8_t   frequency_adjustment = 0; // [-100%, 100%]
            uint32_t flags = 0;
            int16_t  static_attenuation = 0;
            struct {
               uint8_t stop  = 0;
               uint8_t start = 0;
            } times;
            std::array<int16_t, 5> attenuation_curve;
            int16_t reverb_attenuation_control = 0;
            int32_t priority = 0;
            std::array<uint8_t, 8> unknown;
         };

      public:
         components::object_bounds bounds;
         components::papyrus_attachment_data script_data;
         //
         form_reference_t descriptor; // SDSC // value is SNDR
         std::optional<legacy_data> legacy;

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