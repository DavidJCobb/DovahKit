#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/container.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Container : public Form {
      public:
         static constexpr form_type_t form_type = form_type::container;
         Container() : Form(form_type) {};

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
         components::model model;
         localized_string name; // FULL
         components::container_data inventory;
         container_flags_t container_flags = 0;
         float weight = 0.0F;
         form_id_t open_sound;
         form_id_t close_sound;

         void load(tes_record_reader&);
         static void generateUseInfo(tes_record_reader&, form_stub*);
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}