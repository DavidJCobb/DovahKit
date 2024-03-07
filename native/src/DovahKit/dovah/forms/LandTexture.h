#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class LandTexture : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::land_texture;
         LandTexture(const constructor_params& c) : Form(form_type, c) {};

         struct remaster_flag {
            remaster_flag() = delete;
            enum type : uint8_t {
               is_snow = 0x01,
            };
         };
         using remaster_flags_t = std::underlying_type_t<remaster_flag::type>;

         form_reference_t texture_set; // TNAM; should be a TXST
         struct {
            form_reference_t material; // MNAM; should be a MATT
            uint8_t friction    = 0; // CK clamps this to [0, 100]
            uint8_t restitution = 0; // CK clamps this to [0, 200]
         } havok;
         uint8_t specular_exponent = 1; // CK clamps this to [0, 100]
         std::vector<form_reference_t> grasses;
         remaster_flags_t remaster_flags = 0;
         //
         components::papyrus_attachment_data script_data;
         components::object_bounds bounds;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}