#pragma once
#include <bit>
#include <cstdint>
#include <cstring> // memset
#include <limits>
#include <type_traits>

namespace cobb {
   template<uint32_t count> class bitset {
      private:
         using chunk_type = uint32_t;
         static_assert(std::is_unsigned_v<chunk_type>);

         static constexpr int        bits_per_chunk    = sizeof(chunk_type) * 8;
         static constexpr chunk_type all_bits_set      = std::numeric_limits<chunk_type>::max();
         static constexpr int        chunk_count       = count / bits_per_chunk + (count % bits_per_chunk ? 1 : 0);
         //
         // To understand these next constexprs and any other mentions of "partial chunks," 
         // see the comments for find_first_clear.
         //
         static constexpr int        undershoot_cc     = count / bits_per_chunk; // number of non-partial chunks
         static constexpr int        bits_in_partial   = count % bits_per_chunk;
         static constexpr bool       has_partial_chunk = bits_in_partial != 0;
         static constexpr chunk_type partial_chunk_max = (chunk_type(1) << (count % bits_per_chunk)) - 1; // like all_bits_set but for the partial chunk
         
         chunk_type data[chunk_count];

         struct reference {
            friend class bitset;
            private:
               bitset& target;
               size_t  index;

               reference(bitset& b, size_t i) : target(b), index(i) {}

            public:
               inline operator bool() const noexcept {
                  return target.test(index);
               }
               inline bool operator~() const noexcept {
                  return !target.test(index);
               }

               inline reference& operator=(bool x) noexcept {
                  if (x)
                     target.set(index);
                  else
                     target.reset(index);
               }
               inline reference& operator=(const reference& x) noexcept {
                  if ((bool)x)
                     target.set(index);
                  else
                     target.reset(index);
               }
               reference& flip() noexcept {
                  target.flip(index);
               }
         };
         
      public:
         constexpr bitset() {
            this->clear();
         }

         constexpr size_t size() const noexcept {
            return count;
         }
         
         constexpr bool all() const noexcept {
            for (uint32_t i = 0; i < undershoot_cc; i++)
               if (this->data[i] == all_bits_set)
                  return false;
            if constexpr (has_partial_chunk) {
               if (this->data[chunk_count - 1] != partial_chunk_max)
                  return false;
            }
            return true;
         }
         constexpr bool any() const noexcept {
            return !none();
         }
         constexpr bool none() const noexcept {
            //
            // We don't have to worry about partial chunks here, since we memset all chunks 
            // to zero. The unused portions of a partial chunk should always be cleared.
            //
            for (uint32_t i = 0; i < chunk_count; i++)
               if (this->data[i])
                  return false;
            return true;
         }

         constexpr bool test(size_t index) const {
            uint32_t   ci  = index / bits_per_chunk;
            chunk_type bit = chunk_type(1) << (index % bits_per_chunk);
            return this->data[ci] & bit;
         }
         constexpr void set(size_t index) {
            uint32_t   ci  = index / bits_per_chunk;
            chunk_type bit = chunk_type(1) << (index % bits_per_chunk);
            this->data[ci] |= bit;
         }
         constexpr void reset(size_t index) {
            uint32_t   ci  = index / bits_per_chunk;
            chunk_type bit = chunk_type(1) << (index % bits_per_chunk);
            this->data[ci] &= ~bit;
         }
         constexpr void flip(size_t index) {
            uint32_t   ci  = index / bits_per_chunk;
            chunk_type bit = chunk_type(1) << (index % bits_per_chunk);
            auto& chunk = this->data[ci];
            if (chunk & bit)
               chunk &= ~bit;
            else
               chunk |= bit;
         }

         constexpr void clear() noexcept {
            if (std::is_constant_evaluated()) {
               for (size_t i = 0; i < chunk_count; ++i)
                  this->data[i] = 0;
            } else {
               memset(&this->data, 0, sizeof(this->data));
            }
         }
         
         constexpr int32_t find_first_clear() const {
            //
            // Finds the first zero bit in the set. This function performs significantly 
            // better than looping from 0 to (count) and testing each individual bit, as you 
            // would have to do when using std::bitset as of this writing.
            //
            for (uint32_t i = 0; i < undershoot_cc; i++) {
               auto chunk = this->data[i];
               if (chunk != all_bits_set) {
                  return (i * bits_per_chunk) + std::countr_one(chunk);
               }
            }
            if constexpr (has_partial_chunk) {
               //
               // If the number of bits in the set isn't cleanly divisible by 32, then we're 
               // going to have a final chunk that only uses some of its bits. We need to ONLY 
               // LOOK AT THE BITS THAT THAT CHUNK ACTUALLY USES, or we'll end up returning bit 
               // indices past the end of our set.
               //
               auto chunk = this->data[chunk_count - 1];
               if (chunk != partial_chunk_max) {
                  constexpr auto _offset = (chunk_count - 1) * bits_per_chunk;
                  auto j = std::countr_one(chunk);
                  if (j >= bits_in_partial)
                     return -1;
                  return _offset + j;
               }
            }
            return -1;
         }
         constexpr int32_t find_first_clear_from(uint32_t index) const {
            //
            // Finds the first zero bit in the set. This function performs significantly 
            // better than looping from 0 to (count) and testing each individual bit, as you 
            // would have to do when using std::bitset as of this writing.
            //
            uint32_t ci = index / bits_per_chunk; // chunks to skip
            uint8_t  bi = index % bits_per_chunk; // bits   to skip
            {
               auto chunk = this->data[ci] | ((chunk_type(1) << bi) - 1);
               if (chunk != all_bits_set) {
                  return (ci * bits_per_chunk) + std::countr_one(chunk);
               }
            }
            uint32_t i = ci + 1;
            for (; i < undershoot_cc; ++i) {
               auto chunk = this->data[i];
               if (chunk != all_bits_set) {
                  return (i * bits_per_chunk) + std::countr_one(chunk);
               }
            }
            if constexpr (has_partial_chunk) {
               //
               // If the number of bits in the set isn't cleanly divisible by 32, then we're 
               // going to have a final chunk that only uses some of its bits. We need to ONLY 
               // LOOK AT THE BITS THAT THAT CHUNK ACTUALLY USES, or we'll end up returning bit 
               // indices past the end of our set.
               //
               auto chunk = this->data[chunk_count - 1];
               if (ci == undershoot_cc)
                  chunk |= ((1 << ((chunk_type)bi + 1)) - 1);
               if (chunk != partial_chunk_max) {
                  constexpr auto _offset = (chunk_count - 1) * bits_per_chunk;
                  auto j = std::countr_one(chunk);
                  if (j >= bits_in_partial)
                     return -1;
                  return _offset + j;
               }
            }
            return -1;
         }

         template<typename T> requires std::is_integral_v<T>
         constexpr T get_span(size_t offset) const {
            T out = {};
            if (offset % 32 == 0) {
               out = this->data[offset / 32];
            } else {
               size_t n = offset / 32;
               size_t s = offset % 32;

               out = this->data[n] >> s;
               out |= this->data[n + 1] << (32 - s);
            }
            return out;
         }

         reference operator[](size_t i) {
            return reference(*this, i);
         }
         constexpr bool operator[](size_t i) const {
            return this->test(i);
         }
   };
};