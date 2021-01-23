#pragma once
#include <cstdint>
#include <type_traits>

namespace dovah {
   struct story_event_code {
      enum type : uint32_t {
         none = 0, // don't ever serialize this to a file, if you can help it
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


   #if !_DEBUG
   static_assert(false, "Don't forget to actually write a definition and code for the SMEN form type!");
   #endif
}