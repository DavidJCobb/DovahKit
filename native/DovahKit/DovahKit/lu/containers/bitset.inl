#pragma once
#include "./bitset.h"

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#define TEMPLATE_PARAMS template<size_t Bitcount>
#define CLASS_NAME bitset<Bitcount>

namespace lu {
   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::all() const noexcept {
      for (size_t i = 0; i < undershoot_cc; i++)
         if (this->data[i] != all_bits_set)
            return false;
      if constexpr (has_partial_chunk) {
         if (this->data[chunk_count - 1] != partial_chunk_max)
            return false;
      }
      return true;
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::any() const noexcept {
      return !none();
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::none() const noexcept {
      //
      // We don't have to worry about partial chunks here, since we memset all chunks 
      // to zero. The unused portions of a partial chunk should always be cleared.
      //
      for (size_t i = 0; i < chunk_count; i++)
         if (this->data[i])
            return false;
      return true;
   }

   TEMPLATE_PARAMS
   constexpr size_t CLASS_NAME::count() const noexcept {
      size_t result = 0;
      for (size_t i = 0; i < chunk_count; ++i)
         result += std::popcount(this->data[i]);
      return result;
   }

   #pragma region Single-bit operations
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::flip(size_t index) {
         _bounds_check(index);
         size_t     ci = index / bits_per_chunk;
         chunk_type bit = chunk_type(1) << (index % bits_per_chunk);
         this->data[ci] ^= bit;
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::reset(size_t index) {
         _bounds_check(index);
         size_t     ci = index / bits_per_chunk;
         chunk_type bit = chunk_type(1) << (index % bits_per_chunk);
         this->data[ci] &= ~bit;
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::set(size_t index) {
         _bounds_check(index);
         size_t     ci = index / bits_per_chunk;
         chunk_type bit = chunk_type(1) << (index % bits_per_chunk);
         this->data[ci] |= bit;
      }

      TEMPLATE_PARAMS
      constexpr bool CLASS_NAME::test(size_t index) const {
         _bounds_check(index);
         size_t     ci = index / bits_per_chunk;
         chunk_type bit = chunk_type(1) << (index % bits_per_chunk);
         return this->data[ci] & bit;
      }
   #pragma endregion

   #pragma region All-bit operations
      TEMPLATE_PARAMS
      constexpr CLASS_NAME& CLASS_NAME::flip_all() noexcept {
         for (size_t i = 0; i < chunk_count; ++i)
            this->_data[i] = ~this->_data[i];
         if constexpr (has_partial_chunk) {
            this->_data[undershoot_cc] &= partial_chunk_max;
         }
         return *this;
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME& CLASS_NAME::reset_all() noexcept {
         this->clear();
         return *this;
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME& CLASS_NAME::set_all() noexcept {
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
   #pragma endregion
      
   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::clear() noexcept {
      if (std::is_constant_evaluated()) {
         for (size_t i = 0; i < chunk_count; ++i)
            this->data[i] = 0;
      } else {
         memset(&this->data, 0, sizeof(this->data));
      }
   }

   TEMPLATE_PARAMS
   constexpr size_t CLASS_NAME::find_first_clear() const {
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
               return index_of_none;
            return _offset + j;
         }
      }
      return index_of_none;
   }

   TEMPLATE_PARAMS
   constexpr size_t CLASS_NAME::find_first_clear_from(uint32_t index) const {
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
               return index_of_none;
            return _offset + j;
         }
      }
      return index_of_none;
   }
   
   TEMPLATE_PARAMS
   constexpr size_t CLASS_NAME::find_first_set() const {
      for (size_t i = 0; i < chunk_count; i++) {
         auto chunk = this->data[i];
         if (chunk != 0) {
            return (i * bits_per_chunk) + std::countr_zero(chunk);
         }
      }
      return index_of_none;
   }

   TEMPLATE_PARAMS
   constexpr size_t CLASS_NAME::find_first_set_from(uint32_t index) const {
      size_t  ci = index / bits_per_chunk; // chunks to skip
      uint8_t bi = index % bits_per_chunk; // bits   to skip
      {
         auto chunk = this->data[ci] | ((chunk_type(1) << bi) - 1);
         if (chunk != all_bits_set) {
            return (ci * bits_per_chunk) + std::countr_zero(chunk);
         }
      }
      size_t i = ci + 1;
      for (; i < chunk_count; ++i) {
         auto chunk = this->data[i];
         if (chunk != all_bits_set) {
            return (i * bits_per_chunk) + std::countr_zero(chunk);
         }
      }
      return index_of_none;
   }
         
   #pragma region Span operations
      TEMPLATE_PARAMS
      template<typename T> requires std::is_integral_v<T>
      constexpr T CLASS_NAME::get_span(size_t offset) const noexcept {
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

      TEMPLATE_PARAMS
      template<typename T> requires std::is_integral_v<T>
      constexpr void CLASS_NAME::overwrite_span(size_t offset, T span) noexcept {
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
   #pragma endregion
}

#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")