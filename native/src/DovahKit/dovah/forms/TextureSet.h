#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "../../../helpers/unreachable.h"
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/decal_data.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class TextureSet : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::texture_set;
         TextureSet(const constructor_params& c) : Form(form_type, c) {};
         ~TextureSet();

         struct texture_set_flag {
            enum type : uint16_t {
               no_specular_map         = 0x0001,
               is_skin_textures        = 0x0002, // called "Facegen Textures" in the CK, but also used e.g. for elk bodies
               has_model_space_normals = 0x0004,
            };
         };
         using texture_set_flags_t = std::underlying_type_t<texture_set_flag::type>;

         union texture_list {
            std::array<std::string, 8> list = {};
            struct {
               std::string diffuse;          // TX00
               std::string normal;           // TX01 // skin texture sets use this as both normal and gloss
               std::string environment_mask; // TX02 // skin texture sets use this as subsurface tint
               std::string glow_map;         // TX03 // skin texture sets use this as detail map
               std::string height;           // TX04
               std::string cubemap;          // TX05
               std::string multilayer;       // TX06
               std::string backlight;        // TX07
            };

            ~texture_list() {
               for (auto& s : this->list)
                  s.~basic_string();
            }
         };

         texture_list textures;
         texture_set_flags_t texture_flags = 0; // DNAM
         //
         components::decal_data* decal_data = nullptr; // DODT
         components::papyrus_attachment_data script_data;
         components::object_bounds bounds;

         inline bool is_skin_texture_set() const noexcept {
            return (this->texture_flags & texture_set_flag::is_skin_textures) != 0;
         }

         template<size_t s> requires (s < 8)
         inline constexpr std::string& texture_by_index() noexcept {
            auto& list = this->textures;
            if constexpr (s == 0)
               return list.diffuse;
            if constexpr (s == 1)
               return list.normal;
            if constexpr (s == 2)
               return list.environment_mask;
            if constexpr (s == 3)
               return list.glow_map;
            if constexpr (s == 4)
               return list.height;
            if constexpr (s == 5)
               return list.cubemap;
            if constexpr (s == 6)
               return list.multilayer;
            if constexpr (s == 7)
               return list.backlight;
            cobb::unreachable();
         };

         inline std::string* texture_by_index(size_t s) noexcept {
            auto& list = this->textures;
            if (s == 0)
               return &list.diffuse;
            if (s == 1)
               return &list.normal;
            if (s == 2)
               return &list.environment_mask;
            if (s == 3)
               return &list.glow_map;
            if (s == 4)
               return &list.height;
            if (s == 5)
               return &list.cubemap;
            if (s == 6)
               return &list.multilayer;
            if (s == 7)
               return &list.backlight;
            return nullptr;
         };

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