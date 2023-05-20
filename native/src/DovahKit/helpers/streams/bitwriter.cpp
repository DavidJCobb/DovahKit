#include "./bitwriter.h"
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
      void bitwriter::write(const QString& v) {
         auto utf8 = v.toUtf8();

         length_prefix_serialized_type size = utf8.size();
         this->write(size);
         if (size == 0)
            return;

         if (!std::is_constant_evaluated()) {
            if (this->is_byte_aligned()) {
               size_t bytecount = size * q_char_size;
               //
               memcpy(this->_buffer + this->get_bytepos(), utf8.constData(), bytecount);
               this->_position.advance_by_bytes(bytecount);
               return;
            }
         }
         for (uint i = 0; i < size; ++i) {
            uint8_t c = utf8[i];
            this->write(c);
         }
      }
   #endif
}