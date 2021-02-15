#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
#include <vector>
#include "../../helpers/endianness.h"

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

   struct story_event_definition {
      struct member {
         uint16_t    signature;
         const char* bethesda_name;
         const char* name;
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