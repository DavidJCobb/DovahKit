#include "actor_values.h"

namespace {
   struct _range {
      uint8_t min; // AV index (first to fall in this range)
      uint8_t max; // AV index (last to fall in this range)
   };
   _range _ranges[] = {
      //
      // The form ID can be computed from the actor value index. The game uses the same ranges 
      // that we do here, and the same formula once it knows that the AV falls into a range; 
      // it just uses hardcoded bounds checks instead of a loop. See the Skyrim Classic sub-
      // routine at 0x005AD160, though its helper function at 0x005ACFD0 may be easier to read.
      //
      // 0x005AD160: uint32_t ActorValueIndexToFormID(uint32_t index);
      //
      //              - Identifies the range index (i) that the AV falls into.
      //              - Calls _ActorValueOffsetWithinRange to get the offset (o).
      //              - Returns (i + 10) * 100 + o;
      //
      // 0x005ACFD0: uint32_t _ActorValueOffsetWithinRange(uint32_t range, uint32_t index);
      //
      //              - Subtracts from index the minimum end of the range. So for example, 
      //                range 0's minimum is 24, so if the (range) argument is 0, then this 
      //                returns (index - 24).
      //
      //              - This function is easier to read than the other one in my opinion, 
      //                so you can quickly read it to identify all of the ranges and their 
      //                minimums.
      //
      { 24, 37 }, // attributes; attribute heal rates; gameplay-critical stats e.g. speed and carry capacity
      {  6, 23 }, // skills
      {  0,  5 }, // AI values
      { 39, 45 }, // damage resistances
      { 46, 52 }, // limb condition
      { 53, 163 }, // all other actor values
   };
}