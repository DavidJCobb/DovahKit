#pragma once
#include <string>

namespace dovah::pex {
   class tabled_string : public std::string {
      public:
         template<typename Stream>
         constexpr void read(Stream& stream) {
            uint16_t index;
            stream.read(index);
            this->assign(stream.get_tabled_string(index));
         }
         
         template<typename Stream>
         static constexpr void skip(Stream& stream) {
            stream.skip_bytes(2);
         }
   };
}