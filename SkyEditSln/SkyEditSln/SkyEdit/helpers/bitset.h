#pragma once
#include <cstdint>
#include <limits>

namespace cobb {
   template<uint32_t count> class bitset {
      private:
         static constexpr int      bits_per_chunk    = 32;
         static constexpr uint32_t all_bits_set      = std::numeric_limits<uint32_t>::max();
         static constexpr int      chunk_count       = count / bits_per_chunk + (count % bits_per_chunk ? 1 : 0);
         //
         // To understand these next constexprs, see the comments for find_first_clear.
         //
         static constexpr int      undershoot_cc     = count / bits_per_chunk; // number of non-partial chunks
         static constexpr int      bits_in_partial   = count % bits_per_chunk;
         static constexpr bool     has_partial_chunk = bits_in_partial != 0;
         static constexpr int      partial_chunk_max = (1 << (count % bits_per_chunk)) - 1; // like all_bits_set but for the partial chunk
         //
         uint32_t data[chunk_count];
         //
      public:
         bitset() {
            memset(&data, 0, sizeof(data));
         }
         //
         bool none() const {
            for (uint32_t i = 0; i < chunk_count; i++)
               if (data[i])
                  return false;
            return true;
         }
         bool test(uint32_t index) const {
            uint32_t ci  = index / bits_per_chunk;
            uint32_t bit = 1 << (index % bits_per_chunk);
            return data[ci] & bit;
         }
         //
         void set(uint32_t index) {
            uint32_t ci  = index / bits_per_chunk;
            uint32_t bit = 1 << (index % bits_per_chunk);
            data[ci] |= bit;
         }
         void reset(uint32_t index) {
            uint32_t ci  = index / bits_per_chunk;
            uint32_t bit = 1 << (index % bits_per_chunk);
            data[ci] &= ~bit;
         }
         //
         int32_t find_first_clear() const {
            //
            // Finds the first zero bit in the set. This function performs significantly 
            // better than looping from 0 to (count) and testing each individual bit, as you 
            // would have to do when using std::bitset as of this writing.
            //
            for (uint32_t i = 0; i < undershoot_cc; i++) {
               auto chunk = data[i];
               if (chunk != all_bits_set) {
                  for (uint8_t j = 0; j < bits_per_chunk; j++) {
                     if ((chunk & (1 << j)) == 0) {
                        return i * bits_per_chunk + j;
                     }
                  }
               }
            }
            if (has_partial_chunk) {
               //
               // If the number of bits in the set isn't cleanly divisible by 32, then we're 
               // going to have a final chunk that only uses some of its bits. We need to ONLY 
               // LOOK AT THE BITS THAT THAT CHUNK ACTUALLY USES, or we'll end up returning bit 
               // indices past the end of our set.
               //
               auto chunk = data[chunk_count - 1];
               if (chunk != partial_chunk_max) {
                  for (uint8_t j = 0; j < bits_in_partial; j++) {
                     if ((chunk & (1 << j)) == 0) {
                        return (chunk_count - 1) * bits_per_chunk + j;
                     }
                  }
               }
            }
            return -1;
         }
   };
};