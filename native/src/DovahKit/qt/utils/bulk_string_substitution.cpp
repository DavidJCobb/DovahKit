#include "./bulk_string_substitution.h"
#include <cstring>
#include <limits>

namespace dovahkit::qt::utils {
   void bulk_string_substitution::_identify_placeholders() {
      const QChar* data = this->source.constData();
      const size_t size = this->source.size();

      int last_fragment_end = 0;
      int after_last_escape = 0;

      int i = this->source.indexOf(escape_character);
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
            // Get the index of the value to substitute in, e.g. "%0001" -> 1
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
            if (index > std::numeric_limits<param_index_type>::max())
               goto next_escape;

            const size_t placeholder_length = j - i;
            if (placeholder_length > std::numeric_limits<placeholder_length_type>::max())
               goto next_escape;

            {
               auto& list = this->place_indices;
               if (index >= list.size()) {
                  list.resize(index + 1);
               }
               auto& item = list[index];
               if (locale)
                  ++item.count_locale;
               else
                  ++item.count_normal;
            }
            this->placeholder_character_count += placeholder_length;

            this->placeholders.push_back(placeholder{
               .begin        = (position_type)i,
               .length       = (placeholder_length_type)placeholder_length,
               .locale       = locale,
               .replace_with = (param_index_type)index,
            });
         }
         last_fragment_end = j;
      next_escape:
         after_last_escape = j;
         i = this->source.indexOf(escape_character, after_last_escape);
      }
   }

   void bulk_string_substitution::_stringify_substitution(size_t index, QString v) {
      auto& s = this->place_indices[index].stringified;
      s.normal = v; // implicitly share
   }
   void bulk_string_substitution::_stringify_substitution(size_t index, QStringView v) {
      auto& s = this->place_indices[index].stringified;
      s.normal = QString::fromRawData(v.data(), v.size()); // don't copy
   }

   size_t bulk_string_substitution::_result_length() const {
      size_t size = this->source.size() - this->placeholder_character_count;
      for (auto& subst : this->place_indices) {
         size_t size_normal = subst.stringified.normal.size();
         size_t size_locale = subst.stringified.locale.size();
         if (!subst.allow_locale)
            size_locale = size_normal;
         size += subst.count_normal * size_normal;
         size += subst.count_locale * size_locale;
      }
      return size;
   }

   QString bulk_string_substitution::_execute_substitutions() const {
      if (this->placeholders.empty()) {
         return this->source;
      }

      QString result = QString(this->_result_length(), Qt::Uninitialized);
      QChar*  dst    = result.data();

      const auto _insert_replacement = [this, &dst](const placeholder& here) {
         const auto&  subst  = this->place_indices[here.replace_with];
         const QChar* src    = nullptr;
         size_t       length = 0;
         if (here.locale && subst.allow_locale) {
            src    = subst.stringified.locale.unicode();
            length = subst.stringified.locale.size();
         } else {
            src    = subst.stringified.normal.unicode();
            length = subst.stringified.normal.size();
         }
         if (length) {
            memcpy(dst, src, length * sizeof(QChar));
            dst += length;
         }
      };

      {
         const auto& p = this->placeholders[0];
         if (p.begin > 0) {
            memcpy(
               dst,
               this->source.unicode(),
               p.begin * sizeof(QChar)
            );
            dst += p.begin;
         }
         _insert_replacement(p);
      }
      for (size_t i = 1; i < this->placeholders.size(); ++i) {
         const auto& prev = this->placeholders[i - 1];
         const auto& here = this->placeholders[i];

         size_t prev_end       = prev.begin + prev.length;
         size_t between_length = here.begin - prev_end;
         if (between_length) {
            memcpy(
               dst,
               this->source.unicode() + prev_end,
               between_length * sizeof(QChar)
            );
            dst += between_length;
         }

         _insert_replacement(here);
      }
      {
         const auto& p = this->placeholders[this->placeholders.size() - 1];
         size_t end = p.begin + p.length;
         if (end < this->source.size()) {
            size_t after_length = this->source.size() - end;
            memcpy(
               dst,
               this->source.unicode() + end,
               after_length * sizeof(QChar)
            );
            dst += after_length;
         }
      }

      assert(dst == result.constData() + result.size());
      return result;
   }
}