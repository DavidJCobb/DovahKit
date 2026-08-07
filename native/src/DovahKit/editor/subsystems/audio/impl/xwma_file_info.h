#pragma once
#include <cstdint>
struct tWAVEFORMATEX;
struct XAUDIO2_BUFFER;
struct XAUDIO2_BUFFER_WMA;

namespace dovahkit::subsystems::audio::impl {
   class xwma_file_info {
      public:
         constexpr xwma_file_info() {}
         xwma_file_info(const void*, size_t size);

      public:
         const tWAVEFORMATEX* format = nullptr;
         struct {
            const void* data = nullptr;
            uint32_t    size = 0;
         } audio;
         struct {
            const uint32_t* data = nullptr;
            uint32_t        size = 0;
         } dpds;

         constexpr bool valid() const noexcept {
            return this->format && this->audio.data && this->audio.size;
         }

         XAUDIO2_BUFFER describe_buffer() const;
         XAUDIO2_BUFFER_WMA describe_wma() const;
         float estimated_length() const;
   };
}
