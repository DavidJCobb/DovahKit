#pragma once
#include <bit>
#include <cstdint>
#include <cstring> // memset
#include <limits>
#include <stdexcept> // std::out_of_range
#include <type_traits>

namespace cobb {
   template<uint32_t Bitcount>
   class bitset {
      private:
         using chunk_type = uint32_t;
         static_assert(std::is_unsigned_v<chunk_type>);

         static constexpr const size_t bits_per_byte = 8;

         static constexpr const size_t     bits_per_chunk    = sizeof(chunk_type) * bits_per_byte;
         static constexpr const chunk_type all_bits_set      = std::numeric_limits<chunk_type>::max();
         static constexpr const size_t     chunk_count       = Bitcount / bits_per_chunk + (Bitcount % bits_per_chunk ? 1 : 0);

         //
         // The next four variables concern the "partial chunk" that will hold the 
         // last few bits, if the number of bits to store isn't a perfect multiple 
         // of the number of bits per chunk.
         //
         static constexpr const size_t     undershoot_cc     = Bitcount / bits_per_chunk; // number of non-partial chunks
         static constexpr const size_t     bits_in_partial   = Bitcount % bits_per_chunk;
         static constexpr const bool       has_partial_chunk = bits_in_partial != 0;
         static constexpr const chunk_type partial_chunk_max = (chunk_type(1) << (Bitcount % bits_per_chunk)) - 1; // like all_bits_set but for the partial chunk
         
         //
         // Each chunk holds B bits, such that you can access the bit at index N via:
         // 
         //    Bit N == Chunk[N / B] & (1 << (N % B))
         // 
         // In English, higher bit indices are stored closer to the most significant 
         // bit of their containing chunk, while lower bit indices are stored closer 
         // to the least significant bit of their containing chunk; assuming B = 32, 
         // the memory layout will look like this:
         // 
         //    Chunk 0                              Chunk 1
         //    --------------------------------     --------------------------------
         //    N=31                         N=0     N=63                        N=32
         //    01234567012345670123456701234567     01234567012345670123456701234567
         // 
         // If the number of bits in this bitset isn't a multiple of B, then the last 
         // chunk will contain unused ("dead") bits. We force dead bits to zero in 
         // order to avoid having to special-case the last chunk in accessors such as 
         // `any()`, `count()`, and `none()`.
         //
         chunk_type data[chunk_count];

         constexpr void _bounds_check(size_t bit_offset) const {
            #if _DEBUG
            if (bit_offset >= this->size())
               throw std::out_of_range("bit index is out of range");
            #endif
         }

      public:

         // Proxy object which makes it possible for bitset::operator[] to return a 
         // reference to an individual bit, thereby allowing you to assign bits with 
         // statements like `my_bitset[3] = true`. Can only be instantiated by a 
         // bitset instance.
         struct reference {
            friend class bitset;
            private:
               bitset& target;
               size_t  index;

               reference(bitset& b, size_t i) : target(b), index(i) {}

            public:
               constexpr operator bool() const noexcept {
                  return target.test(index);
               }
               constexpr bool operator~() const noexcept {
                  return !target.test(index);
               }

               constexpr reference& operator=(bool x) noexcept {
                  if (x)
                     target.set(index);
                  else
                     target.reset(index);
                  return *this;
               }
               constexpr reference& operator=(const reference& x) noexcept {
                  if ((bool)x)
                     target.set(index);
                  else
                     target.reset(index);
                  return *this;
               }
               reference& flip() noexcept {
                  target.flip(index);
                  return *this;
               }
         };
         
      public:
         constexpr bitset() {
            this->clear();
         }

         // Returns the number of bits that this bitset holds.
         constexpr size_t size() const noexcept {
            return Bitcount;
         }
         
         constexpr bool all() const noexcept {
            for (size_t i = 0; i < undershoot_cc; i++)
               if (this->data[i] != all_bits_set)
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
            for (size_t i = 0; i < chunk_count; i++)
               if (this->data[i])
                  return false;
            return true;
         }

         // Returns the number of bits that are set to true.
         constexpr size_t count() const noexcept {
            size_t result = 0;
            for (size_t i = 0; i < chunk_count; ++i)
               result += std::popcount(this->data[i]);
            return result;
         }

         // Returns the value of the bit at the given index.
         constexpr bool test(size_t index) const {
            _bounds_check(index);
            size_t     ci  = index / bits_per_chunk;
            chunk_type bit = chunk_type(1) << (index % bits_per_chunk);
            return this->data[ci] & bit;
         }

         // Sets the bit at the given index to true.
         constexpr void set(size_t index) {
            _bounds_check(index);
            size_t     ci  = index / bits_per_chunk;
            chunk_type bit = chunk_type(1) << (index % bits_per_chunk);
            this->data[ci] |= bit;
         }

         // Sets the bit at the given index to false.
         constexpr void reset(size_t index) {
            _bounds_check(index);
            size_t     ci  = index / bits_per_chunk;
            chunk_type bit = chunk_type(1) << (index % bits_per_chunk);
            this->data[ci] &= ~bit;
         }

         // Inverts the bit at the given index.
         constexpr void flip(size_t index) {
            _bounds_check(index);
            size_t     ci  = index / bits_per_chunk;
            chunk_type bit = chunk_type(1) << (index % bits_per_chunk);
            auto& chunk = this->data[ci];
            if (chunk & bit)
               chunk &= ~bit;
            else
               chunk |= bit;
         }

         // Sets all bits in the bitset to true.
         constexpr bitset& set_all() noexcept {
            if (std::is_constant_evaluated()) {
               for (size_t i = 0; i < chunk_count; ++i)
                  this->data[i] = all_bits_set;
            } else {
               memset(&this->data, 0xFF, sizeof(this->data));
            }
            if constexpr (has_partial_chunk) {
               this->_data[undershoot_cc] &= partial_chunk_max;
            }
            return *this;
         }

         // Inverts all bits in the bitset.
         constexpr bitset& flip_all() noexcept {
            for (size_t i = 0; i < chunk_count; ++i)
               this->_data[i] = ~this->_data[i];
            if constexpr (has_partial_chunk) {
               this->_data[undershoot_cc] &= partial_chunk_max;
            }
            return *this;
         }

         // Sets all bits in the bitset to false.
         constexpr bitset& reset_all() noexcept {
            this->clear();
            return *this;
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
            for (size_t i = 0; i < undershoot_cc; i++) {
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
            size_t  ci = index / bits_per_chunk; // chunks to skip
            uint8_t bi = index % bits_per_chunk; // bits   to skip
            {
               auto chunk = this->data[ci] | ((chunk_type(1) << bi) - 1);
               if (chunk != all_bits_set) {
                  return (ci * bits_per_chunk) + std::countr_one(chunk);
               }
            }
            size_t i = ci + 1;
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
         
         // Extract a span of bits from the bitset, wherein the size of the span is 
         // determined by the bitcount of the type you specify. Bits in the span will 
         // be ordered such that higher bit indices within the bitset correspond to 
         // more-significant bit indices within the span.
         // 
         // The `offset` parameter indicates the first (lowest index) bit to retrieve.
         // 
         // If your span would pass the end of the bitset, the extra bits will be 
         // set to zero.
         //
         template<typename T> requires std::is_integral_v<T>
         constexpr T get_span(size_t offset) const noexcept {
            constexpr const size_t span_bitcount = sizeof(T) * 8;

            T out = {};
            if (offset >= this->size())
               return out;

            if constexpr (sizeof(T) > sizeof(chunk_type)) {
               constexpr const size_t span_bytecount = sizeof(T);

               size_t i = 0; // measured in bytes
               for(; i + sizeof(chunk_type) - 1 < span_bytecount; i += sizeof(chunk_type)) {
                  chunk_type c = get_span<chunk_type>(offset + i * bits_per_byte);
                  out |= (T)c << (i * 8);
               }
               for(; i < span_bytecount; ++i) {
                  auto byte = get_span<uint8_t>(offset + i * bits_per_byte);
                  out |= (T)byte << (i * bits_per_byte);
               }
               return out;
            }

            if (offset % bits_per_chunk == 0) {
               out = this->data[offset / bits_per_chunk];
            } else {
               size_t n = offset / bits_per_chunk;
               size_t s = offset % bits_per_chunk;

               out = this->data[n] >> s;
               if (n + 1 < chunk_count)
                  out |= this->data[n + 1] << (bits_per_chunk - s);
            }
            return out;
         }

         // Overwrite a span of bits within the bitset, wherein the size of the span 
         // is determined by the bitcount of the type you specify. More-significant 
         // bit indices in your input span map to higher bit indices in the bitset.
         // 
         // The `offset` parameter indicates the first (lowest index) bit to overwrite.
         // 
         // If your span would pass the end of the bitset, the extra bits will be 
         // ignored.
         //
         template<typename T> requires std::is_integral_v<T>
         constexpr void overwrite_span(size_t offset, T span) noexcept {
            if (offset >= this->size())
               return;
            
            constexpr const size_t span_bitcount = sizeof(T) * 8;
            size_t span_end = offset + span_bitcount;

            if constexpr (sizeof(T) > sizeof(chunk_type)) {
               size_t i = 0; // measured in bytes
               for(; i + sizeof(chunk_type) - 1 < sizeof(T); i += sizeof(chunk_type)) {
                  chunk_type subspan = span >> (i * bits_per_byte);
                  overwrite_span<chunk_type>(offset + i * bits_per_byte, subspan);
               }
               for(; i < sizeof(chunk_type); ++i) {
                  uint8_t subspan = span >> (i * bits_per_byte);
                  overwrite_span<uint8_t>(offset + i * bits_per_byte, subspan);
               }
               return;
            }

            chunk_type all_of_T = std::numeric_limits<std::make_unsigned_t<T>>::max();
            if (offset % bits_per_chunk == 0) {
               auto& dst = this->data[offset / bits_per_chunk];
               dst &= ~all_of_T;
               dst |= span;
            } else {
               size_t n = offset / bits_per_chunk;
               size_t s = offset % bits_per_chunk;

               size_t end_bit_rel = sizeof(T) * 8 + s;

               if (end_bit_rel > bits_per_chunk) {
                  this->data[n] &= ~(all_of_T << s);
                  this->data[n] |=  (span << s);
                  if (span_end > this->size()) {
                     if (n + 1 < chunk_count) {
                        auto& dst = this->data[n + 1];
                        dst &= ~(all_of_T >> (bits_per_chunk - s));
                        dst |=  (span >> (bits_per_chunk - s));
                     }
                  }
               } else {
                  auto& dst = this->data[n];
                  dst &= ~(all_of_T << s);
                  dst |=  (span << s);
               }
            }
            if constexpr (bits_in_partial > 0) {
               if (span_end > this->size()) {
                  //
                  // If we're writing to the last chunk(s), ensure that we haven't 
                  // written to the dead bits in our partial chunk -- or rather, 
                  // force those bits back to zero if we *did* write to them.
                  //
                  this->_data[undershoot_cc] &= partial_chunk_max;
               }
            }
         }

         reference operator[](size_t i) {
            return reference(*this, i);
         }
         constexpr bool operator[](size_t i) const {
            return this->test(i);
         }

         constexpr bool operator==(const bitset& other) const noexcept = default;
   };
};