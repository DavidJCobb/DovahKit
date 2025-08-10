#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class LensFlare : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::lens_flare;
         LensFlare(const constructor_params& c) : Form(form_type, c) {};

         struct sprite {
            public:
               struct flag {
                  enum type : uint32_t {
                     rotates               = 1 << 0,
                     shrinks_when_occluded = 1 << 1,
                  };
               };
               using flags_t = std::underlying_type_t<flag::type>;

            public:
               std::string id;      // DNAM
               std::string texture; // FNAM
               struct {
                  struct {
                     float r = 1;
                     float g = 1;
                     float b = 1;
                  } tint;
                  float   width        = 0.1;
                  float   height       = 0.1;
                  float   position     = 1;
                  float   angular_fade = 0;
                  float   opacity      = 1;
                  flags_t flags        = flag::rotates;
               } data; // LFSD
         };

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         float color_influence            = 0; // CNAM
         float fade_distance_radius_scale = 1; // DNAM
         std::vector<sprite> sprites; // LFSP+*[]

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}