#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "./Form.h"
#include "./_common.h"
#include "./components/model.h"
#include "./components/papyrus.h"
#include "helpers/vector3.h"

namespace dovah::loaded_forms {
   class MaterialObject : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::material_object;
         MaterialObject(const constructor_params& c) : Form(form_type, c) {};

         struct flag {
            enum type : uint32_t {
               single_pass = 1 << 0,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

         struct flag_ex {
            enum type : uint8_t {
               snow = 1 << 0,
            };
         };
         using flags_ex_t = std::underlying_type_t<flag_ex::type>;

         using embedded_nif_data = std::vector<uint8_t>;

      public:
         components::model model; // MODL, MODT
         components::papyrus_attachment_data script_data; // VMAD
         //
         struct {
            flags_t    flags    = 0;
            flags_ex_t flags_ex = 0; // SSE-only
            struct {
               float scale = 1;
               float bias  = 0;
            } falloff;
            float material_uv_scale = 1;
            float noise_uv_scale    = 1;
            float normal_dampener   = 0;
            cobb::vector3<float> projection_vector;
            struct {
               float r = 0;
               float g = 0;
               float b = 0;
            } single_pass_color;
         } directional_material;

         //
         // TODO: These are embedded binary streams each containing a NiProperty (and any 
         //       other blocks it may reference, e.g. BSShaderTextureSets). The Creation 
         //       Kit pulls them from the form's model and writes them directly into the 
         //       form data. The model isn't used for any other purpose.
         // 
         //       This means that in order to let users properly and fully edit this form, 
         //       you must be able to load AND SAVE NIF data -- not just round-tripping 
         //       files, but saving specific blocks and their dependencies. DovahKit cannot 
         //       do this at this time, so changing the model path must be prohibited until 
         //       then.
         // 
         //       Once we implement the ability to save NIF data, we'll need to come back, 
         //       have these load as proper NIFs, and update the form functions (e.g. the 
         //       "clone" function) to manage them appropriately.
         //
         std::vector<embedded_nif_data> properties;

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