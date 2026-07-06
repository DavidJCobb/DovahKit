#pragma once
#include <cstdint>
#include <type_traits>
#include <QLocale>
#include <QString>
#include "helpers/small_vector.h"

namespace dovahkit::qt::utils {
   namespace impl::_bulk_string_substitution {
      template<typename T>
      concept floating_point_like = std::is_floating_point_v<T> || std::is_same_v<std::decay_t<T>, qfloat16>;

      template<typename T>
      concept integral_non_char = (
         (std::is_integral_v<T> || (std::is_enum_v<T> && std::is_scoped_enum_v<T>)) &&
         !floating_point_like<T> &&
         !std::is_convertible_v<T, QAnyStringView>
      );

      template<typename T>
      concept formattable_as_numeric = floating_point_like<T> || integral_non_char<T>;

      template<typename T>
      concept locale_stringifiable = requires(T v, QLocale locale) {
         { locale.toString(v) };
      };
   }

   //
   // Similar to a series of calls to QString::arg, except for two differences:
   // 
   //  - All replacements are performed at once.
   // 
   //  - QString::arg replaces the lowest-numbered substitution marker, which 
   //    makes it completely unsuitable for situations where replacement markers 
   //    are optional (e.g. a string which may or may not have "%3", but will 
   //    have "%4"; if "%3" is missing, "%4" will be replaced with the third 
   //    substitution, and the fourth substitution will be lost).
   // 
   //    By contrast, this helper always maps the values you pass in to their 
   //    indices, even if an index has no marker present in the string. If you 
   //    pass four arguments and your string only has %1, %2, and %4 in it, 
   //    those will substitute to arguments 1, 2, and 4, not 1, 2, and 3.
   // 
   // Current implementation limits:
   // 
   //  - Strings whose lengths can't bit in a four-byte integer are unsupported.
   // 
   //  - Placeholder indices above 65535 can't be replaced.
   // 
   //  - No more than 65535 instances of a single placeholder index can be 
   //    replaced at a time.
   //
   class bulk_string_substitution {
      protected:
         using count_type              = uint16_t;
         using placeholder_length_type = uint8_t;
         using position_type           = uint32_t;
         using param_index_type        = uint16_t;

         static constexpr const QChar escape_character = '%';

         // By convention, QString substitution tokens begin at %1, not %0.
         static constexpr const size_t minimum_marker_index = 1;

         struct placeholder_index {
            count_type count_normal = 0;
            count_type count_locale = 0; // e.g. occurrences of "%L1"
            bool       allow_locale = false;
            struct {
               QString normal; // maybe-owning; if value to substitute is a string, this will be a view (via QString::fromRawData), not a copy
               QString locale; // always owning; created from scratch via QLocale
            } stringified;
         };

         struct placeholder {
            position_type           begin  = 0;
            placeholder_length_type length = 0;
            bool                    locale = false;
            param_index_type        replace_with = 0;
         };

         QString source;
         cobb::small_vector<placeholder_index, 4, true> place_indices;
         cobb::small_vector<placeholder,       4, true> placeholders; // in order from earliest (within source string) to latest
         size_t placeholder_character_count = 0; // total length in QChars of all to-be-replaced markers in the source string

         void _identify_placeholders();

         #pragma region _stringify_substitution
            void _stringify_substitution(size_t index, QString);
            void _stringify_substitution(size_t index, QStringView);

            template<typename T>
               requires (
                  impl::_bulk_string_substitution::formattable_as_numeric<T> &&
                  impl::_bulk_string_substitution::locale_stringifiable<T>
               )
            void _stringify_substitution(size_t index, T v) {
               auto& subst = this->place_indices[index];
               if (subst.count_normal) {
                  auto locale = QLocale::c();
                  locale.setNumberOptions(QLocale::OmitGroupSeparator);
                  subst.stringified.normal = locale.toString(v);
               }
               if (subst.count_locale) {
                  subst.stringified.locale = QLocale().toString(v);
               }
               subst.allow_locale = true;
            }

            template<typename T>
               requires (
                  impl::_bulk_string_substitution::formattable_as_numeric<T> &&
                  !impl::_bulk_string_substitution::locale_stringifiable<T> &&
                  std::is_convertible_v<T, std::conditional_t<std::is_signed_v<T>, qlonglong, qulonglong>>
               )
            void _stringify_substitution(size_t index, T v) {
               this->_stringify_substitution(index, std::conditional_t<std::is_signed_v<T>, qlonglong, qulonglong>(v));
            }
         #pragma endregion

         size_t _result_length() const;
         QString _execute_substitutions() const;

      public:
         bulk_string_substitution(QStringView source) : source(source) {
            this->_identify_placeholders();
         }

         template<typename... Args>
         QString exec(Args&&... args) {
            size_t i = 0;
            (
               (i < this->place_indices.size() ?
                  this->_stringify_substitution(i, std::forward<Args>(args)),++i
               :
                  ++i
               ),
               ...
            );
            return this->_execute_substitutions();
         }
   };
}