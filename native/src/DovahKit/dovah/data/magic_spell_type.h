#pragma once
#include <cstdint>

namespace dovah {
   //
   // This is the enum identified as `MagicSystem::SpellType` in CommonLibSSE. 
   // This is mostly only used at run-time, to allow the game to check or refer 
   // to subclasses of `MagicItem` without dynamic casting or `typeid`; for 
   // example, a `CureEffect` indicates whether it cures addictions, diseases, 
   // or poisons using this enum. There are a few cases where specific values 
   // from this enum are stored in form data, though.
   //
   enum class magic_spell_type : uint32_t {
      spell,              // legal for SPEL/SPIT+0x04
      disease,            // legal for SPEL/SPIT+0x04
      power_greater,      // legal for SPEL/SPIT+0x04
      power_lesser,       // legal for SPEL/SPIT+0x04
      ability,            // legal for SPEL/SPIT+0x04
      poison,             // legal for SPEL/SPIT+0x04
      enchantment_normal, // legal for ENCH/ENIT+0x14
      alchemy_item,       // hardcoded for ALCH form type
      ingredient,         // hardcoded for INGR form type
      leveled_spell,      // hardcoded for LVSP form type
      addiction,          // legal for SPEL/SPIT+0x04
      power_voice,        // legal for SPEL/SPIT+0x04
      enchantment_staves, // legal for ENCH/ENIT+0x14
      scroll,             // hardcoded for SCRL form type
   };
}