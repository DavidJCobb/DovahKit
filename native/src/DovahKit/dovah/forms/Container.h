#pragma once
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/container.h"
#include "components/destruction.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Container : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::container;
         Container(const constructor_params& c) : Form(form_type, c) {};

         struct container_flag {
            container_flag() = delete;
            enum type : uint8_t {
               animation_allows_sounds = 0x01,
               respawns                = 0x02,
               show_owner              = 0x04,
            };
         };
         using container_flags_t = std::underlying_type_t<container_flag::type>;

         components::papyrus_attachment_data script_data;
         components::object_bounds bounds;
         components::model_ts model;
         std::optional<components::destruction_stage_data> destruction_data; // DEST
         localized_string name; // FULL
         components::container_data inventory;
         container_flags_t container_flags = 0;
         float             weight          = 0.0F;
         form_reference_t  open_sound;
         form_reference_t  close_sound;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}