#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/decal_data.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class TextureSet : public Form {
      #include "impl/form_subclass_components.txt"
      public:
         static constexpr form_type_t form_type = form_type::texture_set;
         TextureSet(const constructor_params& c) : Form(form_type, c) {};

         struct texture_set_flag {
            enum type : uint16_t {
               no_specular_map         = 0x0001,
               facegen_textures        = 0x0002,
               has_model_space_normals = 0x0004,
            };
         };
         using texture_set_flags_t = std::underlying_type_t<texture_set_flag::type>;

         struct {
            std::string diffuse;
            std::string normal;   // or gloss
            std::string env_mask; // or subsurface tint
            std::string detail;   // or glow map
            std::string height;
            std::string environment;
            std::string multilayer;
            std::string specular; // or backlight mask
         } textures;
         texture_set_flags_t texture_flags = 0;
         //
         components::decal_data decal_data;
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