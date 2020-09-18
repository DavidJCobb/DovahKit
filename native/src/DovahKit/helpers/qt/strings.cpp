#include "strings.h"

namespace cobb::qt {
   QString four_cc_to_string(uint32_t signature) {
      return QString("%1%2%3%4")
         .arg(QChar(signature >> 0x18))
         .arg(QChar((signature >> 0x10) & 0xFF))
         .arg(QChar((signature >> 0x08) & 0xFF))
         .arg(QChar(signature & 0xFF));
   }
}