#include "asset_manager.h"
#include <QRegularExpression>
#include <QStringView>
#include "../../helpers/cpuinfo.h"

DovahKitAssetManager::DovahKitAssetManager() {
}

namespace {
   QString _normalizePathComponent(const QStringView& view) {
      uint    size = view.size();
      QString text;
      text.resize(size);
      //
      uint i = 0;
      if (size >= 8) {
         static bool can_intrin = ([]() {
            auto& cpu = cobb::cpuinfo::get();
            return cpu.extension_support.sse_2 && cpu.extension_support.sse_3;
         })();
         if (can_intrin) {
            auto* src = view.data();
            auto* dst = text.data();
            //
            auto mb_a = _mm_set1_epi8('A' - 1);
            auto mb_z = _mm_set1_epi8('Z' + 1);
            for (; i + 15 < size; i += 16) {
               auto ma = _mm_loadu_si128((const __m128i*)(src + i));
               //
               // Goal: for each active byte in (mask_a), OR the byte in (ma) by 0x20
               //
               auto mask_a = _mm_cmpgt_epi8(ma, mb_a); // per byte: (a >= 'A') ? 0xFF : 0
               auto mask_z = _mm_cmplt_epi8(ma, mb_z); // per byte: (a <= 'Z') ? 0xFF : 0
               mask_a = _mm_and_si128(mask_a, mask_z); // bitwise-AND
               mask_a = _mm_and_si128(_mm_set1_epi8(0x20), mask_a); // per byte: (a >= 'A' && a <= 'Z') ? 0x20 : 0
               ma = _mm_or_si128(ma, mask_a); // bitwise-OR
               //
               _mm_storeu_si128((__m128i*)(dst + i), ma);
            }
            if (i + 7 < size) {
               auto ma = _mm_loadl_epi64((const __m128i*)(src + i));
               //
               auto mask_a = _mm_cmpgt_epi8(ma, mb_a); // per byte: (a >= 'A') ? 0xFF : 0
               auto mask_z = _mm_cmplt_epi8(ma, mb_z); // per byte: (a <= 'Z') ? 0xFF : 0
               mask_a = _mm_and_si128(mask_a, mask_z); // bitwise-AND
               mask_a = _mm_and_si128(_mm_set1_epi8(0x20), mask_a); // per byte: (a >= 'A' && a <= 'Z') ? 0x20 : 0
               ma = _mm_or_si128(ma, mask_a); // bitwise-OR
               //
               _mm_storel_epi64((__m128i*)(dst + i), ma);
               //
               i += 8;
            }
         }
      }
      for (; i < size; ++i) {
         auto c = view[i];
         auto u = c.unicode();
         if (u >= 'A' && u <= 'Z') {
            c = QChar::fromLatin1(u + 0x20);
         }
         text[i] = c;
      }
      return text;
   }
}
/*static*/ QString DovahKitAssetManager::normalizeAssetPath(const QString& base) {
   QString path;
   if (base.isEmpty())
      return path;
   int i    = 0;
   int size = base.size();
   if (base[0] == '/' || base[0] == '\\')
      ++i;
   //
   auto sep_ex = QRegularExpression("/\\");
   //
   int prev = -1;
   for (int j = path.indexOf(sep_ex, i); j >= 0; i = j + 1, j = path.indexOf(sep_ex, i)) {
      auto component = QStringView(base).mid(i, j - i);
      if (component.isEmpty()) {
         prev = i;
         continue;
      }
      if (component == '.') {
         continue;
      }
      if (component == QLatin1Literal("..")) {
         if (prev < 0)
            return QString();
         path = path.left(prev);
         prev = path.lastIndexOf('/');
         continue;
      }
      if (!path.isEmpty())
         path += '/';
      path += _normalizePathComponent(component);
      prev  = i;
   }
   return path;
}