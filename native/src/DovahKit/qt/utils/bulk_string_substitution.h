#pragma once
#include <type_traits>
#include <variant>
#include <vector>
#include <QLocale>
#include <QString>

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
   class bulk_string_substitution {
      protected:
         static constexpr const QChar  marker_start = '%';
         static constexpr const size_t no_marker    = (size_t)-1;

         // By convention, QString substitution tokens begin at %1, not %0.
         static constexpr const size_t minimum_marker_index = 1;

         struct marker_index_info {
            size_t count_normal = 0;
            size_t count_locale = 0; // e.g. "%L1"
            size_t total_length = 0; // e.g. strlen("%99") + strlen("%00099")
            struct {
               std::variant<QStringView, QString> normal;
               QString locale;
               bool allow_locale = false;
            } stringified;
         };

         // Tracks the area between two markers, and indices the index of the 
         // marker that follows that area.
         //
         // If a marker appears at the very start of the string, there'll be a 
         // zero-length fragment indicating its presence. If a marker doesn't 
         // appear at the end of the string, there'll be a fragment with the 
         // "no marker" marker index.
         struct fragment {
            struct {
               size_t begin = 0;
               size_t end   = 0;
            } span_before;
            size_t marker_index = no_marker;
            bool   locale       = false;
         };

         QString source;
         std::vector<marker_index_info> marker_stats;
         std::vector<fragment> fragments;

         void _identify_markers();

         #pragma region _stringify_substitution
            void _stringify_substitution(size_t index, QStringView);

            template<typename T>
               requires (
                  impl::_bulk_string_substitution::formattable_as_numeric<T> &&
                  impl::_bulk_string_substitution::locale_stringifiable<T>
               )
            void _stringify_substitution(size_t index, T v) {
               auto& marker = this->marker_stats[index];
               if (marker.count_normal) {
                  auto locale = QLocale::c();
                  locale.setNumberOptions(QLocale::OmitGroupSeparator);
                  marker.stringified.normal = locale.toString(v);
               }
               if (marker.count_locale) {
                  marker.stringified.locale = QLocale().toString(v);
               }
               marker.stringified.allow_locale = true;
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
            this->_identify_markers();
         }

         template<typename... Args>
         QString exec(Args&&... args) {
            size_t i = 0;
            (
               (i < this->marker_stats.size() ?
                  this->_stringify_substitution(i, args),++i
               :
                  ++i
               ),
               ...
            );
            return this->_execute_substitutions();
         }
   };
}