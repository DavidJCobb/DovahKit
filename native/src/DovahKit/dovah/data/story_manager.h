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

   union story_event_member_code {
      char     c[2];
      uint16_t i;

      story_event_member_code() {}
      story_event_member_code(uint16_t x) : i(cobb::to_big_endian(x)) {}
      story_event_member_code(char a, char b) : c{a, b} {}
   };
   union story_event_member_code_w {
      char     c[4];
      uint32_t i = 0xFFFFFFFF; // same sentinel used by the game

      story_event_member_code_w() {}
      story_event_member_code_w(uint16_t x) : i(cobb::to_big_endian(x)) {}
      story_event_member_code_w(uint32_t x) : i(cobb::to_big_endian(x)) {}
      story_event_member_code_w(char a, char b) : c{ a, b } {}

      inline bool operator==(const story_event_member_code& other) const noexcept {
         return this->c[0] == other.c[0] && this->c[1] == other.c[1];
      }
   };

   struct story_event_definition {
      struct member {
         story_event_member_code signature;
         const char* name;
         const char* bethesda_name;
      };

      story_event_code_t signature;
      std::vector<member> members;

      static std::array<story_event_definition, 31> list;
      static story_event_definition* lookup(story_event_code_t);
   };
}