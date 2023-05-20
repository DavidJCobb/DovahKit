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
         std::string utf8;
         this->read(utf8);

         v = QString::fromUtf8(utf8.data(), utf8.size());
      }
   #endif
}