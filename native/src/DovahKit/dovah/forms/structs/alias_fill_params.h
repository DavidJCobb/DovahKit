#pragma once
#include <cstdint>
#include <optional>
#include <variant>
#include "dovah/data/story_manager.h"
#include "dovah/form_reference_t.h"

namespace dovah::loaded_forms {
   using alias_id_t = uint32_t;
}

namespace dovah::loaded_forms::structs {
   namespace alias_fill_params {
      constexpr const alias_id_t no_alias = -1;

      struct copy_external_alias {
         form_reference_t quest;            // ALEQ
         alias_id_t       alias = no_alias; // ALEA/ALFA
      };
      struct event_params {
         story_event_code_t code   = story_event_code::undefined; // ALFE
         uint32_t           member = 0; // ALFD
      };
      
      namespace loc {
         struct preassigned {
            form_reference_t location;
         };
         struct at_reference_alias {
            alias_id_t       alias = no_alias;
            form_reference_t keyword;
         };
         struct find {
            std::optional<event_params> from_event;
         };
      }
      namespace ref {
         struct preassigned {
            form_reference_t ref;
         };
         struct unique_actor {
            form_reference_t actor_base;
         };
         struct at_location_alias {
            alias_id_t       alias = no_alias;
            form_reference_t loc_ref_type;
         };
         struct create {
            form_reference_t base_form;
            uint32_t         difficulty;
            struct {
               alias_id_t alias = no_alias;
               bool       place_in_inventory;
            } at_reference;
         };
         struct find_in_loaded_area { // serialized as alias flag 0x00000020
            bool closest = false; // serialized as alias flag 0x00002000
         };
         struct find_from_event : public event_params {};
         struct find_near_alias {
            alias_id_t alias     = no_alias;
            uint32_t   near_type = 0;
         };
      }
   }
   
   using location_alias_fill_params = std::variant<
      alias_fill_params::loc::find, // executable-level default
      alias_fill_params::loc::preassigned,
      alias_fill_params::loc::at_reference_alias,
      alias_fill_params::copy_external_alias
   >;
   using reference_alias_fill_params = std::variant<
      alias_fill_params::ref::preassigned,
      alias_fill_params::ref::unique_actor,
      alias_fill_params::ref::at_location_alias,
      alias_fill_params::copy_external_alias,
      alias_fill_params::ref::create,
      alias_fill_params::ref::find_in_loaded_area,
      alias_fill_params::ref::find_from_event,
      alias_fill_params::ref::find_near_alias
   >;

   struct location_alias_fill_params_use_info_state {
      size_t    which = 0;
      form_id_t preassigned;
      form_id_t external_quest;
      form_id_t keyword;
   };
   struct reference_alias_fill_params_use_info_state {
      size_t    which = 0;
      form_id_t preassigned;
      form_id_t unique_actor;
      form_id_t external_quest;
      form_id_t loc_ref_type;
      form_id_t create_base_form;
   };
}