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
               is_skin_textures        = 0x0002, // called "Facegen Textures" in the CK, but also used e.g. for elk bodies
               has_model_space_normals = 0x0004,
            };
         };
         using texture_set_flags_t = std::underlying_type_t<texture_set_flag::type>;

         struct {
            std::string diffuse;          // TX00
            std::string normal;           // TX01 // skin texture sets use this as both normal and gloss
            std::string environment_mask; // TX02 // skin texture sets use this as subsurface tint
            std::string glow_map;         // TX03 // skin texture sets use this as detail map
            std::string height;           // TX04
            std::string cubemap;          // TX05
            std::string multilayer;       // TX06
            std::string backlight;        // TX07
         } textures;
         texture_set_flags_t texture_flags = 0; // DNAM
         //
         components::decal_data decal_data; // DODT
         components::papyrus_attachment_data script_data;
         components::object_bounds bounds;

         inline bool is_skin_texture_set() const noexcept {
            return (this->texture_flags & texture_set_flag::is_skin_textures) != 0;
         }

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