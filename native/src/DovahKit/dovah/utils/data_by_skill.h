#pragma once
#include <array>
#include "../data/skills.h"

namespace dovah {
   template<typename ValueType>
   union data_by_skill {
      public:
         using value_type = ValueType;

         #pragma region Union boilerplate
         constexpr data_by_skill() {}
         //
         // NOTE: Omitting the const-other constructor causes an internal compiler error 
         //       on MSVC as of 8/14/2024.
         //
         #if __cpp_lib_is_within_lifetime
            constexpr data_by_skill(const data_by_skill& other) {
               if (std::is_within_lifetime(&other.one_handed)) {
                  this->list = {
                     other.one_handed,
                     other.two_handed,
                     other.archery,
                     other.block,
                     other.smithing,
                     other.heavy_armor,
                     other.light_armor,
                     other.pickpocket,
                     other.lockpicking,
                     other.sneak,
                     other.alchemy,
                     other.speech,
                     other.alteration,
                     other.conjuration,
                     other.destruction,
                     other.illusion,
                     other.restoration,
                     other.enchanting,
                  };
               } else {
                  this->list = other.list;
               }
            }
         #else
            data_by_skill(const data_by_skill& other) : list(other.list) {}
         #endif

         constexpr ~data_by_skill() {
            this->list.~array();
         }
         #pragma endregion

      public:
         struct {
            value_type one_handed;
            value_type two_handed;
            value_type archery;
            value_type block;
            value_type smithing;    // NPCs never craft on their own, though it's not impossible that a mod might read this and use it.
            value_type heavy_armor;
            value_type light_armor;
            value_type pickpocket;  // only used by NPCs to scale how difficult it is for the player to pickpocket them; NPCs cannot pickpocket others.
            value_type lockpicking; // not used by NPCs; they only lockpick when commanded to, and in that case will always succeed.
            value_type sneak;
            value_type alchemy;     // NPCs never craft on their own, though it's not impossible that a mod might read this and use it.
            value_type speech;      // not used by NPCs.
            value_type alteration;
            value_type conjuration;
            value_type destruction;
            value_type illusion;
            value_type restoration;
            value_type enchanting;  // NPCs never craft on their own, though it's not impossible that a mod might read this and use it.
         };
         std::array<value_type, 18> list = {};
   };
}