#include "./bitreader.h"
#include <cstdint>
#ifdef QT_CORE_LIB
   #include <QChar>
   #include <QString>
#endif

namespace {
   constexpr const size_t q_char_size = sizeof(QChar);
   using q_std_char = std::conditional_t<
      q_char_size == 1,
      char,
      std::conditional_t<
         q_char_size == 2,
         wchar_t,
         std::conditional_t<
            q_char_size == 4,
            uint32_t,
            uint64_t
         >
      >
   >;
}

namespace cobb::streams {
   #ifdef QT_CORE_LIB
      void bitreader::read(QString& v) {
         size_t size;
         this->read(size);
         v.resize(size);
         if (size == 0)
            return;

         if (!std::is_constant_evaluated()) {
            if (this->is_byte_aligned()) {
               size_t bytecount = size * q_char_size;
               size_t data_end  = this->get_bytepos() + bytecount;
               if (data_end <= this->size()) {
                  memcpy(v.data(), this->_buffer + this->get_bytepos(), size * bytecount);
                  this->_position.advance_by_bytes(bytecount);
                  //
                  return;
               }
            }
         }
         for (uint i = 0; i < size; ++i) {
            q_std_char c;
            this->read(c);
            v[i] = c;
         }
      }
   #endif
}