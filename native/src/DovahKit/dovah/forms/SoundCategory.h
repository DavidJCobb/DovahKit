#pragma once
#include <cstdint>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class SoundCategory : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::sound_category;
         SoundCategory(const constructor_params& c) : Form(form_type, c) {};

         struct sound_category_flag {
            enum type : uint32_t {
               mute_when_submerged = 0x01,
               show_in_audio_menu  = 0x02,
            };
         };
         using sound_category_flags_t = std::underlying_type_t< sound_category_flag::type>;

         components::papyrus_attachment_data script_data;
         //
         localized_string       name;      // FULL
         sound_category_flags_t flags = 0; // FNAM
         form_reference_t       parent;    // PNAM // another SNCT form
         uint16_t               static_volume_mult = 0xFFFF; // 0xFFFF = 100% // VNAM (or SNAM encoded as a float)
         uint16_t               default_menu_value = 0xFFFF; // 0xFFFF = 100% // UNAM

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