#include "./bulk_string_substitution.h"
#include <cstring>
#include <limits>
#include <QLocale>

namespace dovahkit::qt::utils {
   void bulk_string_substitution::_identify_markers() {
      const QChar* data = this->source.constData();
      const size_t size = this->source.size();

      int last_fragment_end = 0;
      int after_last_escape = 0;

      int i = this->source.indexOf(marker_start);
      while (i >= 0 && i + 1 < size) {
         size_t begin  = i;
         bool   locale = false;

         int   j = i + 1;
         QChar c = data[j];
         if (c == 'L') {
            locale = true;
            if (++j == size)
               break;
            c = data[j];
         }
         int digit = c.digitValue();
         if (digit == -1)
            goto next_escape;

         {
            int index = digit;
            while (++j < size) {
               c     = data[j];
               digit = c.digitValue();
               if (digit == -1)
                  break;
               index = (index * 10) + digit;
            }

            if (index < minimum_marker_index)
               goto next_escape;
            index -= minimum_marker_index;
            if (index >= std::numeric_limits<stored_marker_index_type>::max())
               goto next_escape;

            {  // Marker stats by index
               auto& list = this->marker_stats;
               if (index >= list.size()) {
                  list.resize(index + 1);
               }
               auto& item = list[index];
               if (locale)
                  ++item.count_locale;
               else
                  ++item.count_normal;
            }
            this->marker_character_count += (j - i);

            // Fragment
            auto& frag = this->fragments.emplace_back();
            frag.span_before  = { (position_type)last_fragment_end, (position_type)i };
            frag.marker_index = index;
            frag.locale       = locale;
         }
         last_fragment_end = j;
      next_escape:
         after_last_escape = j;
         i = this->source.indexOf(marker_start, after_last_escape);
      }
      if (last_fragment_end < size) {
         auto& frag = this->fragments.emplace_back();
         frag.span_before  = { (position_type)last_fragment_end, (position_type)size };
         frag.marker_index = no_marker;
      }
   }

   void bulk_string_substitution::_stringify_substitution(size_t index, QStringView v) {
      auto& s = this->marker_stats[index].stringified;
      s.normal = v;
   }

   size_t bulk_string_substitution::_result_length() const {
      size_t size = this->source.size() - this->marker_character_count;
      for (auto& marker : this->marker_stats) {
         size_t size_normal = 0;
         size_t size_locale = marker.stringified.locale.size();
         if (std::holds_alternative<QStringView>(marker.stringified.normal)) {
            size_normal = std::get<QStringView>(marker.stringified.normal).size();
         } else {
            size_normal = std::get<QString>(marker.stringified.normal).size();
         }
         size += marker.count_normal * size_normal;
         size += marker.count_locale * size_locale;
      }
      return size;
   }

   QString bulk_string_substitution::_execute_substitutions() const {
      auto result = QString(this->_result_length(), Qt::Uninitialized);

      QChar* dst = result.data();
      for (auto& fragment : this->fragments) {
         if (fragment.span_before.end > fragment.span_before.begin) {
            size_t frag_length = fragment.span_before.end - fragment.span_before.begin;
            memcpy(
               dst,
               this->source.unicode() + fragment.span_before.begin,
               frag_length * sizeof(QChar)
            );
            dst += frag_length;
         }
         if (fragment.marker_index != no_marker) {
            const auto&  marker = this->marker_stats[fragment.marker_index];
            const QChar* src    = nullptr;
            size_t       length = 0;
            if (fragment.locale && marker.stringified.allow_locale) {
               src    = marker.stringified.locale.unicode();
               length = marker.stringified.locale.size();
            } else {
               const auto& v = marker.stringified.normal;
               if (std::holds_alternative<QStringView>(v)) {
                  const auto& casted = std::get<QStringView>(v);
                  src    = casted.data();
                  length = casted.size();
               } else {
                  const auto& casted = std::get<QString>(v);
                  src    = casted.unicode();
                  length = casted.size();
               }
            }
            if (length) {
               memcpy(dst, src, length * sizeof(QChar));
               dst += length;
            }
         }
      }
      assert(dst == result.constData() + result.size());
      return result;
   }
}