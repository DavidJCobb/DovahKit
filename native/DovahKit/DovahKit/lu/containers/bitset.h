#pragma once
#include <bit>
#include <cstdint>
#include <cstring> // memset
#include <limits>
#include <stdexcept> // std::out_of_range
#include <type_traits>

namespace lu {
   template<size_t Bitcount>
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
         
         constexpr bool all() const noexcept;
         constexpr bool any() const noexcept;
         constexpr bool none() const noexcept;

         // Returns the number of bits that are set to true.
         constexpr size_t count() const noexcept;

         #pragma region Single-bit operations
            // Inverts the bit at the given index.
            constexpr void flip(size_t index);

            // Sets the bit at the given index to false.
            constexpr void reset(size_t index);

            // Sets the bit at the given index to true.
            constexpr void set(size_t index);

            // Returns the value of the bit at the given index.
            constexpr bool test(size_t index) const;
         #pragma endregion

         #pragma region All-bit operations
            // Inverts all bits in the bitset.
            constexpr bitset& flip_all() noexcept;

            // Sets all bits in the bitset to false.
            constexpr bitset& reset_all() noexcept;

            // Sets all bits in the bitset to true.
            constexpr bitset& set_all() noexcept;
         #pragma endregion

         constexpr void clear() noexcept;
         
         // Finds the first zero bit in the set. This function performs significantly better 
         // than looping from 0 to `count` and testing each bit by hand, as you would have to 
         // do when using `std::bitset`.
         constexpr int32_t find_first_clear() const;

         // Finds the first zero bit in the set. This function performs significantly better 
         // than looping from `index` to `count` and testing each bit by hand, as you would 
         // have to do when using `std::bitset`.
         constexpr int32_t find_first_clear_from(uint32_t index) const;

         constexpr int32_t find_first_set() const;

         constexpr int32_t find_first_set_from(uint32_t index) const;
         
         #pragma region Span operations
            // Extract a span of bits from the bitset, wherein the size of the span is 
            // determined by the bitcount of the type you specify. Bits in the span will 
            // be ordered such that higher bit indices within the bitset correspond to 
            // more-significant bit indices within the span.
            // 
            // The `offset` parameter indicates the first (lowest index) bit to retrieve.
            // 
            // If your span would pass the end of the bitset, the extra bits will be 
            // set to zero.
            template<typename T> requires std::is_integral_v<T>
            constexpr T get_span(size_t offset) const noexcept;

            // Overwrite a span of bits within the bitset, wherein the size of the span 
            // is determined by the bitcount of the type you specify. More-significant 
            // bit indices in your input span map to higher bit indices in the bitset.
            // 
            // The `offset` parameter indicates the first (lowest index) bit to overwrite.
            // 
            // If your span would pass the end of the bitset, the extra bits will be 
            // ignored.
            template<typename T> requires std::is_integral_v<T>
            constexpr void overwrite_span(size_t offset, T span) noexcept;
         #pragma endregion

         constexpr bool operator[](size_t i) const & {
            return this->test(i);
         }
         reference operator[](size_t i) & {
            return reference(*this, i);
         }

         constexpr bool operator==(const bitset& other) const noexcept = default;
   };
};

#include "./bitset.inl"