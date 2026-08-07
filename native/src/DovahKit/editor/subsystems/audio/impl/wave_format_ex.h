#pragma once
#include <cstdint>
struct pcmwaveformat_tag;
struct tWAVEFORMATEX;
struct waveformat_tag;

//
// This header just exists so we can use WAVEFORMATEX and friends without having to 
// include Windows headers and all their cruft.
//

#pragma pack(push,1) // don't pad these structs

namespace dovahkit::subsystems::audio::impl {
   struct wave_format {
      static constexpr const size_t serialized_size = 14;

      uint16_t wFormatTag;
      uint16_t nChannels;
      uint32_t nSamplesPerSec;
      uint32_t nAvgBytesPerSec;
      uint16_t nBlockAlign;

      constexpr wave_format& operator=(const waveformat_tag& v) noexcept {
         *this = *(const wave_format*)&v;
         return *this;
      }
   };
   static_assert(offsetof(wave_format, nBlockAlign) + sizeof(wave_format::nBlockAlign) == wave_format::serialized_size);
   static_assert(sizeof(wave_format) == wave_format::serialized_size);

   struct pcm_wave_format {
      static constexpr const size_t serialized_size = 16;

      // inheritance won't work, because `wave_format` might get padded
      union {
         wave_format wf;
         struct {
            uint16_t wFormatTag;
            uint16_t nChannels;
            uint32_t nSamplesPerSec;
            uint32_t nAvgBytesPerSec;
            uint16_t nBlockAlign;
            uint16_t wBitsPerSample;
         };
      };

      constexpr pcm_wave_format& operator=(const pcmwaveformat_tag& v) noexcept {
         *this = *(const pcm_wave_format*)&v;
         return *this;
      }
   };
   static_assert(offsetof(pcm_wave_format, wBitsPerSample) + sizeof(pcm_wave_format::wBitsPerSample) == pcm_wave_format::serialized_size);
   static_assert(sizeof(pcm_wave_format) == pcm_wave_format::serialized_size);

   struct wave_format_ex {
      static constexpr const size_t serialized_size = 18;

      uint16_t wFormatTag;
      uint16_t nChannels;
      uint32_t nSamplesPerSec;
      uint32_t nAvgBytesPerSec;
      uint16_t nBlockAlign;
      uint16_t wBitsPerSample; // for a single channel
      uint16_t cbSize; // size of extra-data after this header

      constexpr wave_format_ex& operator=(const wave_format& v) noexcept {
         *(wave_format*)this = *(const wave_format*)&v;
         this->wBitsPerSample = 0;
         this->cbSize         = 0;
         if (v.wFormatTag == 1) { // WAVE_FORMAT_PCM
            this->wBitsPerSample = (v.nBlockAlign / v.nChannels) * 8;
         }
         return *this;
      }
      constexpr wave_format_ex& operator=(const pcm_wave_format& v) noexcept {
         *this = v.wf;
         this->wBitsPerSample = v.wBitsPerSample;
         return *this;
      }
      constexpr wave_format_ex& operator=(const tWAVEFORMATEX& v) noexcept {
         *this = *(const wave_format_ex*)&v;
         return *this;
      }
   };
   static_assert(offsetof(wave_format_ex, cbSize) + sizeof(wave_format_ex::cbSize) == wave_format_ex::serialized_size);
   static_assert(sizeof(wave_format_ex) == wave_format_ex::serialized_size);
}

#pragma pack(pop)