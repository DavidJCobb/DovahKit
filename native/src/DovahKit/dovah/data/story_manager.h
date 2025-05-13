#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
#include <vector>
#include "../core.h"

namespace dovah {

   //
   // Serialization code should ensure that story event codes and member codes are big-endian. 
   // For the backend file code, write_signature should do.
   //

   struct story_event_code {
      enum type : uint32_t {
         none      = 0,          // don't ever serialize this to a file, if you can help it
         undefined = 0xFFFFFFFF, // don't ever serialize this to a file, if you can help it // sentinel value used by TESV.exe within aliases
         //
         crime_gold               = 'ADCR',
         actor_dialogue           = 'ADIA',
         player_activate_actor    = 'AFAV',
         actor_hello              = 'AHEL',
         player_add_item          = 'AIPL',
         arrest                   = 'ARRT',
         assault                  = 'ASSU',
         bribe                    = 'BRIB',
         cast_magic               = 'CAST',
         change_relationship_rank = 'CHRR',
         change_location          = 'CLOC',
         craft_item               = 'CRFT',
         player_cured             = 'CURE',
         dead_body                = 'DEAD',
         escaped_jail             = 'ESJA',
         flatter                  = 'FLAT',
         player_infected          = 'INFC',
         intimidate               = 'INTM',
         jail                     = 'JAIL',
         kill                     = 'KILL',
         level_up                 = 'LEVL',
         lockpick                 = 'LOCK',
         new_voice_power          = 'NVPE',
         pay_fine                 = 'PFIN',
         player_receives_favor    = 'PRFV',
         player_remove_item       = 'REMP',
         quest_start              = 'QSTR',
         script                   = 'SCPT',
         skill_increase           = 'SKIL',
         served_time_in_jail      = 'STIJ',
         trespass                 = 'TRES',
      };
   };
   using story_event_code_t = std::underlying_type_t<story_event_code::type>;

   inline constexpr const auto all_story_event_codes = std::array{
      story_event_code::none,
      story_event_code::undefined,
      story_event_code::crime_gold,
      story_event_code::actor_dialogue,
      story_event_code::player_activate_actor,
      story_event_code::actor_hello,
      story_event_code::player_add_item,
      story_event_code::arrest,
      story_event_code::assault,
      story_event_code::bribe,
      story_event_code::cast_magic,
      story_event_code::change_relationship_rank,
      story_event_code::change_location,
      story_event_code::craft_item,
      story_event_code::player_cured,
      story_event_code::dead_body,
      story_event_code::escaped_jail,
      story_event_code::flatter,
      story_event_code::player_infected,
      story_event_code::intimidate,
      story_event_code::jail,
      story_event_code::kill,
      story_event_code::level_up,
      story_event_code::lockpick,
      story_event_code::new_voice_power,
      story_event_code::pay_fine,
      story_event_code::player_receives_favor,
      story_event_code::player_remove_item,
      story_event_code::quest_start,
      story_event_code::script,
      story_event_code::skill_increase,
      story_event_code::served_time_in_jail,
      story_event_code::trespass,
   };

   struct story_event_definition {
      struct member {
         uint16_t    signature;
         const char* bethesda_name;
         const char* name;
         std::vector<form_type> allowed_form_types; // if this list contains only (form_type::none), then treat it as "any form type usable by the GetIsID condition." a non-empty list implies that the member is a form.
         //
         constexpr bool is_form() const noexcept { return !this->allowed_form_types.empty(); }
         constexpr bool can_only_be_reference() const noexcept {
            auto& list = this->allowed_form_types;
            if (list.size() == 1 && form_type_is_reference(list[0]))
               return true;
            return false;
         }
      };

      story_event_code_t signature;
      std::vector<member> members;

      const member* member_by_signature(uint16_t) const;
      const member* member_by_wide_signature(uint32_t) const;

      static const std::array<story_event_definition, 31> list;
      static const story_event_definition* lookup(story_event_code_t);

      inline static uint32_t widen_member_code(uint16_t s) noexcept { return s << 0x10; }
   };
}