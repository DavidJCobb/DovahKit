#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/keyword_list.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Activator : public Form {
      public:
         static constexpr form_type_t form_type = form_type::activator;
         Activator() : Form(form_type) {};

         struct activator_flag {
            activator_flag() = delete;
            enum type : uint16_t {
               no_displacement    = 0x0001,
               ignored_by_sandbox = 0x0002,
            };
         };
         using activator_flags_t = std::underlying_type_t<activator_flag::type>;

         components::papyrus_attachment_data papyrus;
         components::object_bounds bounds;
         components::model model;
         components::destruction_stage_data destruction_data;
         components::keyword_list keywords;
         localized_string name; // FULL
         struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t alpha; // unused
         } marker_color; // CNAM
         form_id_t looping_sound;
         form_id_t activation_sound;
         form_id_t water_type;
         form_id_t interact_keyword;
         localized_string  activation_verb;
         activator_flags_t activator_flags;

         void load(tes_record_reader&);
         static void generateUseInfo(tes_record_reader&, form_stub*);
   };
}