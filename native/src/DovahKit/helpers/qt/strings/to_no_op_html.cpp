#include "./to_no_op_html.h"
#include <string_view>
#include <utility>
#include <QRegularExpression>

namespace cobb::qt::strings {
   extern QString to_no_op_html(QString src, std::string_view wrap_in_tag) {
      constexpr const std::string_view tag_start = "<qt>";
      constexpr const std::string_view tag_end   = "</qt>";

      constexpr const std::string_view entity_amp = "&amp;";
      constexpr const std::string_view entity_lt  = "&lt;";
      constexpr const std::string_view entity_gt  = "&gt;";

      QString dst;
      if (wrap_in_tag.empty()) {
         dst.reserve(src.size() + tag_start.size() + tag_end.size());
         dst.append(QLatin1String(tag_start.data(), tag_start.size()));
      } else {
         dst.reserve(src.size() + 5 + wrap_in_tag.size() * 2);
         dst.append('<');
         dst.append(QLatin1String(wrap_in_tag.data(), wrap_in_tag.size()));
         dst.append('>');
      }

      {
         auto re = QRegularExpression("[<>&]");

         size_t from = 0;
         size_t size = src.size();
         do {
            auto i = src.indexOf(re, from);
            if (i < 0)
               break;
            
            if (i > from)
               dst += src.mid(from, i - from);

            QChar c = src[i];
            switch (c.unicode()) {
               case '<':
                  dst += QLatin1String(entity_lt.data(), entity_lt.size());
                  break;
               case '>':
                  dst += QLatin1String(entity_gt.data(), entity_gt.size());
                  break;
               case '&':
                  dst += QLatin1String(entity_amp.data(), entity_amp.size());
                  break;
               default:
                  std::unreachable();
            }

            from = i + 1;
            if (from >= size)
               break;
         } while (true);
         if (from < size)
            dst += src.mid(from);
      }
      
      if (wrap_in_tag.empty()) {
         dst.append(QLatin1String(tag_end.data(), tag_end.size()));
      } else {
         dst.append("</");
         dst.append(QLatin1String(wrap_in_tag.data(), wrap_in_tag.size()));
         dst.append('>');
      }
      return dst;
   }
}