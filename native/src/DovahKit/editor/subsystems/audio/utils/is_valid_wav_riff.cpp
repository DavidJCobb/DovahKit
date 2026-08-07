#include "./is_valid_wav_riff.h"
#include "./parse_riff.h"
#include "../impl/wave_format_ex.h"

namespace dovahkit::subsystems::audio::utils {
   extern bool is_valid_wav_riff(const void* data, size_t size) {
      bool     valid             = true;
      uint32_t minimum_data_size = 0;

      bool is_relevant_riff = utils::parse_riff(
         data,
         size,
         'WAVE',
         [&valid, &minimum_data_size](uint32_t chunk_type, const void* chunk_data, size_t chunk_size) -> utils::parse_riff_result {
            switch (chunk_type) {
               case 'fmt ':
                  if (chunk_size < impl::wave_format::serialized_size) {
                     valid = false;
                     return utils::parse_riff_result::stop;
                  }
                  {
                     auto* format = (const impl::wave_format*)chunk_data;
                     if (format->nChannels < 1)
                        goto bad;
                     minimum_data_size = format->nBlockAlign;
                     if (chunk_size >= impl::wave_format_ex::serialized_size) {
                        auto* format = (const impl::wave_format_ex*)chunk_data;
                        if (format->nBlockAlign == 0) {
                           if (format->wBitsPerSample == 0)
                              goto bad;
                           minimum_data_size = (format->wBitsPerSample / 8) + ((format->wBitsPerSample % 8) ? 1 : 0);
                        }
                     } else {
                        if (format->nBlockAlign == 0)
                           goto bad;
                        minimum_data_size = format->nBlockAlign;
                     }
                  }
                  break;
               case 'data':
                  if (chunk_size < minimum_data_size)
                     goto bad;
                  break;
               default:
                  break;
            }
            return utils::parse_riff_result::proceed;
         bad:
            valid = false;
            return utils::parse_riff_result::stop;
         }
      );
      return is_relevant_riff && valid;
   }
}