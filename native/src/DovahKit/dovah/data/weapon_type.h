#pragma once

namespace dovah {
   enum class weapon_type {
      hand_to_hand_melee,
      one_hand_sword,
      one_hand_dagger,
      one_hand_axe,
      one_hand_mace,
      two_hand_sword,  greatsword = two_hand_sword,
      two_hand_axe,    battleaxe  = two_hand_axe,   warhammer = two_hand_axe,
      bow,
      staff,
      crossbow,
   };
}
