#pragma once
#include <array>
#include <cstdint>
#include <intrin.h>
#include "cpuinfo.h"

namespace cobb::mask_operations {
   namespace impl {
      template<typename T> concept is_bool_list_type = requires (T x) {
         { (const bool)x[0] };
         { x.size() } -> std::same_as<size_t>;
      };
      template<auto value> concept is_bool_list = is_bool_list_type<std::decay_t<decltype(value)>>;

      template<typename T> concept type_is_16_byte_ready = requires {
         requires sizeof(T) <= 16;
         requires sizeof(T) == 16 || sizeof(T) == 8 || sizeof(T) == 4 || sizeof(T) == 2 || sizeof(T) == 1;
      };

      template<auto mask> requires is_bool_list<mask>
      consteval auto pblendvb_bool_list_to_bytes() {
         std::array<uint8_t, std::max<size_t>(16, mask.size())> out = {};
         size_t i = 0;
         for (; i < mask.size(); ++i)
            out[i] = mask[i] ? 0x80 : 0x00;
         for (; i < out.size(); ++i)
            out[i] = 0x00;
         return out;
      }

      template<auto mask> inline __m128i pblendvb_bool_list_to_m128i() {
         constexpr auto x = pblendvb_bool_list_to_bytes<mask>();
         return _mm_set_epi8(x[15], x[14], x[13], x[12], x[11], x[10], x[9], x[8], x[7], x[6], x[5], x[4], x[3], x[2], x[1], x[0]);
      }

      // Changes the mask length to at least 16 bytes, and ensures that it matches the 
      // native endianness given sizeof(T).
      template<typename T, auto mask> requires (is_bool_list<mask> && mask.size() >= sizeof(T))
      consteval auto fixup_pblendvb_bool_list() {
         constexpr auto final_length = std::max<size_t>(16, mask.size());
         //
         std::array<bool, final_length> out = {};
         size_t i = 0;
         for (; i < mask.size(); ++i) {
            if constexpr (sizeof(T) > 1 && std::endian::native == std::endian::little) {
               auto offset = i % sizeof(T);
               out[i] = mask[(i / sizeof(T) * sizeof(T)) + (sizeof(T) - offset - 1)];
            } else {
               out[i] = mask[i];
            }
         }
         for (; i < final_length; ++i)
            out[i] = out[i % sizeof(T)];
         return out;
      }
   }

   //
   // Given a destination pointer, a source pointer, a length, and an array of bools, iterates over bytes 
   // in the destination. If the Nth bool is true, then the Nth byte in the destination will be replaced 
   // with the Nth byte in the source. If the array of bools is not long enough to fill all of the data, 
   // then it will "loop" back around to its beginning.
   //
   template<auto mask> requires impl::is_bool_list<mask>
   void overwrite_bytes_with_mask(void* a, const void* b, size_t size) {
      auto* cast_a = (uint8_t*)a;       // for easier pointer arithmetic
      auto* cast_b = (const uint8_t*)b; // for easier pointer arithmetic
      //
      size_t i = 0;
      if (cpuinfo::get().extension_support.sse_4_1) {
         if constexpr (mask.size() <= 16) {
            //
            // For small masks, we can do this all in one go.
            //
            constexpr __m128i converted_mask = impl::pblendvb_bool_list_to_m128i<impl::fixup_pblendvb_bool_list<uint8_t, mask>()>();
            for (; i + 15 < size; i += 16) {
               //
               // std::array<uint8_t, 16> a;
               // std::array<uint8_t, 16> b;
               // std::array<uint8_t, 16> converted_mask = { 0x80, 0x80, ... };
               // for(uint8_t i = 0; i < 16; ++i)
               //    if (mask[i] & 0x80)
               //       a[i] = b[i];
               //
               auto ma = _mm_loadu_si128((const __m128i*)(cast_a + i));
               auto mb = _mm_loadu_si128((const __m128i*)(cast_b + i));
               ma = _mm_blendv_epi8(ma, mb, converted_mask);
               _mm_storeu_si128((__m128i*)(cast_a + i), ma);
            }
         } else {
            //
            // For large masks, things are a bit more involved.
            //
            constexpr auto mask_as_bytes = impl::pblendvb_bool_list_to_bytes<mask>();
            for (; i + 15 < size; i += 16) {
               auto ma = _mm_loadu_si128((const __m128i*)(cast_a + i));
               auto mb = _mm_loadu_si128((const __m128i*)(cast_b + i));
               auto mm = _mm_set_epi8(
                  mask_as_bytes[i + 15],
                  mask_as_bytes[i + 14],
                  mask_as_bytes[i + 13],
                  mask_as_bytes[i + 12],
                  mask_as_bytes[i + 11],
                  mask_as_bytes[i + 10],
                  mask_as_bytes[i +  9],
                  mask_as_bytes[i +  8],
                  mask_as_bytes[i +  7],
                  mask_as_bytes[i +  6],
                  mask_as_bytes[i +  5],
                  mask_as_bytes[i +  4],
                  mask_as_bytes[i +  3],
                  mask_as_bytes[i +  2],
                  mask_as_bytes[i +  1],
                  mask_as_bytes[i +  0]
               );
               ma = _mm_blendv_epi8(ma, mb, mm);
               _mm_storeu_si128((__m128i*)(cast_a + i), ma);
            }
         }
      }
      //
      // If the input length wasn't a multiple of 16 bytes, or if SSE 4.1 intrinsics 
      // are not available, then wrap up what remains manually:
      //
      for (; i < mask.size(); ++i)
         if (mask[i])
            *(cast_a + i) = *(cast_b + i);
      for (; i < size; ++i)
         if (mask[i % mask.size()])
            *(cast_a + i) = *(cast_b + i);
   }

   //
   // Given a destination pointer, a source pointer, a length, and an array of bools, iterates over bytes 
   // in the destination. If the Nth bool is true, then the Nth byte in the destination will be replaced 
   // with the Nth byte in the source. If the array of bools is not long enough to fill all of the data, 
   // then it will "loop" back around to its beginning.
   // 
   // This overload takes typed pointers, and reorders bools in the mask to match the platform endianness. 
   // As an example:
   // 
   //    uint32_t a = 0x11223344;
   //    uint32_t b = 0xAABBCCDD;
   //    overwrite_values_with_mask<std::array{false,true,false,true}>(&a, &b, 1);
   // 
   // Reordering ensures that the bool array produces the expected result (0x11BB33DD) on little-endian 
   // systems.
   //
   template<typename T, auto mask> requires impl::is_bool_list<mask>
   void overwrite_values_with_mask(T* a, const T* b, size_t count) {
      auto* cast_a = (uint8_t*)a;       // for easier pointer arithmetic
      auto* cast_b = (const uint8_t*)b; // for easier pointer arithmetic
      //
      size_t i = 0;
      size_t size = count * sizeof(T);
      constexpr auto final_mask = impl::fixup_pblendvb_bool_list<T, mask>();
      if (cpuinfo::get().extension_support.sse_4_1) {
         if constexpr (mask.size() <= 16) {
            //
            // For small masks, we can do this all in one go.
            //
            constexpr __m128i converted_mask = impl::pblendvb_bool_list_to_m128i<final_mask>();
            for (; i + 15 < size; i += 16) {
               //
               // std::array<uint8_t, 16> a;
               // std::array<uint8_t, 16> b;
               // std::array<uint8_t, 16> mask = { 0x80, 0x80, ... };
               // for(uint8_t i = 0; i < 16; ++i)
               //    if (mask[i] & 0x80)
               //       a[i] = b[i];
               //
               auto ma = _mm_loadu_si128((const __m128i*)(cast_a + i));
               auto mb = _mm_loadu_si128((const __m128i*)(cast_b + i));
               ma = _mm_blendv_epi8(ma, mb, converted_mask);
               _mm_storeu_si128((__m128i*)(cast_a + i), ma);
            }
         } else {
            //
            // For large masks, things are a bit more involved.
            //
            constexpr auto mask_as_bytes = impl::pblendvb_bool_list_to_bytes<final_mask>();
            for (; i + 15 < size; i += 16) {
               auto ma = _mm_loadu_si128((const __m128i*)(cast_a + i));
               auto mb = _mm_loadu_si128((const __m128i*)(cast_b + i));
               auto mm = _mm_set_epi8(
                  mask_as_bytes[i + 15],
                  mask_as_bytes[i + 14],
                  mask_as_bytes[i + 13],
                  mask_as_bytes[i + 12],
                  mask_as_bytes[i + 11],
                  mask_as_bytes[i + 10],
                  mask_as_bytes[i +  9],
                  mask_as_bytes[i +  8],
                  mask_as_bytes[i +  7],
                  mask_as_bytes[i +  6],
                  mask_as_bytes[i +  5],
                  mask_as_bytes[i +  4],
                  mask_as_bytes[i +  3],
                  mask_as_bytes[i +  2],
                  mask_as_bytes[i +  1],
                  mask_as_bytes[i +  0]
               );
               ma = _mm_blendv_epi8(ma, mb, mm);
               _mm_storeu_si128((__m128i*)(cast_a + i), ma);
            }
         }
      }
      //
      // If the input length wasn't a multiple of 16 bytes, or if SSE 4.1 intrinsics 
      // are not available, then wrap up what remains manually:
      //
      for(; i < final_mask.size(); ++i)
         if (final_mask[i])
            *(cast_a + i) = *(cast_b + i);
      for (; i < size; ++i)
         if (final_mask[i % final_mask.size()])
            *(cast_a + i) = *(cast_b + i);
   }

   //
   // Given a destination pointer, a source value, a length, and an array of bools, iterates over bytes 
   // in the destination. If the Nth bool is true, then the Nth byte in the destination will be replaced 
   // with the Nth byte in the source. If the array of bools is not long enough to fill all of the data, 
   // then it will "loop" back around to its beginning; similarly, if N exceeds the size of the source 
   // value, then it "loops" to the start.
   // 
   // This overload takes typed pointers, and reorders bools in the mask to match the platform endianness. 
   // As an example:
   // 
   //    std::array<uint32_t, 2> a = { 0x11223344, 0x55667788 };
   //    uint32_t b = 0xAABBCCDD;
   //    overwrite_values_with_mask<std::array{false,true,false,true}>(&a, b, 2);
   // 
   // Reordering ensures that the bool array produces the expected result on little-endian systems:
   // 
   //    { 0x11BB33DD, 0x55BB77DD }
   //
   template<typename T, auto mask> requires (impl::is_bool_list<mask> && impl::type_is_16_byte_ready<T>)
   void overwrite_values_with_mask(T* a, const T b, size_t count) {
      auto* cast_a = (uint8_t*)a; // for easier pointer arithmetic
      std::array<uint8_t, 16> b_source;
      {
         constexpr size_t rep = 16 / sizeof(T);
         for (size_t i = 0; i < rep; ++i)
            for (size_t j = 0; j < sizeof(T); ++j)
               b_source[j + i * sizeof(T)] = *((uint8_t*)&b + j);
      }
      //
      size_t i    = 0;
      size_t size = count * sizeof(T);
      constexpr auto final_mask = impl::fixup_pblendvb_bool_list<T, mask>();
      if (cpuinfo::get().extension_support.sse_4_1) {
         if constexpr (mask.size() <= 16) {
            //
            // For small masks, we can do this all in one go.
            //
            __m128i converted_mask = impl::pblendvb_bool_list_to_m128i<final_mask>();
            for (; i + 15 < size; i += 16) {
               //
               // std::array<uint8_t, 16> a;
               // std::array<uint8_t, 16> b;
               // std::array<uint8_t, 16> mask = { 0x80, 0x80, ... };
               // for(uint8_t i = 0; i < 16; ++i)
               //    if (mask[i] & 0x80)
               //       a[i] = b[i];
               //
               auto ma = _mm_loadu_si128((const __m128i*)(cast_a + i));
               auto mb = _mm_loadu_si128((const __m128i*)b_source.data());
               ma = _mm_blendv_epi8(ma, mb, converted_mask);
               _mm_storeu_si128((__m128i*)(cast_a + i), ma);
            }
         } else {
            //
            // For large masks, things are a bit more involved.
            //
            constexpr auto mask_as_bytes = impl::pblendvb_bool_list_to_bytes<final_mask>();
            for (; i + 15 < size; i += 16) {
               auto ma = _mm_loadu_si128((const __m128i*)(cast_a + i));
               auto mb = _mm_loadu_si128((const __m128i*)b_source.data());
               auto mm = _mm_set_epi8(
                  mask_as_bytes[i + 15],
                  mask_as_bytes[i + 14],
                  mask_as_bytes[i + 13],
                  mask_as_bytes[i + 12],
                  mask_as_bytes[i + 11],
                  mask_as_bytes[i + 10],
                  mask_as_bytes[i +  9],
                  mask_as_bytes[i +  8],
                  mask_as_bytes[i +  7],
                  mask_as_bytes[i +  6],
                  mask_as_bytes[i +  5],
                  mask_as_bytes[i +  4],
                  mask_as_bytes[i +  3],
                  mask_as_bytes[i +  2],
                  mask_as_bytes[i +  1],
                  mask_as_bytes[i +  0]
               );
               ma = _mm_blendv_epi8(ma, mb, mm);
               _mm_storeu_si128((__m128i*)(cast_a + i), ma);
            }
         }
      }
      //
      // If the input length wasn't a multiple of 16 bytes, or if SSE 4.1 intrinsics 
      // are not available, then wrap up what remains manually:
      //
      for(; i < final_mask.size(); ++i)
         if (final_mask[i])
            *(cast_a + i) = b_source[i % 16];
      for (; i < size; ++i)
         if (final_mask[i % final_mask.size()])
            *(cast_a + i) = b_source[i % 16];
   }
}