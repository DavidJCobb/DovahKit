#include "./is_valid_fuz_file.h"
#include "dovah/files/fuz/file_info.h"
#include "./is_valid_xwma_riff.h"

namespace dovahkit::subsystems::audio::utils {
   extern bool is_valid_fuz_file(const void* data, size_t size) {
      if (!dovah::fuz::file_info::data_is_fuz(data, size))
         return false;
      auto fuz_info = dovah::fuz::file_info{ data, size };
      if (size > fuz_info.buffer.size + dovah::fuz::header_size) {
         auto*    riff_data = (const void*)((const uint8_t*)data + dovah::fuz::header_size + fuz_info.buffer.size);
         uint32_t riff_size = size - fuz_info.buffer.size - dovah::fuz::header_size;
         return utils::is_valid_xwma_riff(riff_data, riff_size);
      }
      return true;
   }
}