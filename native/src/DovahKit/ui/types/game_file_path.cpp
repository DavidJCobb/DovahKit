#include "./game_file_path.h"

static bool _is_directory_separator(QChar c) {
   return c == '/' || c == '\\';
}

static void _strip_leading_directory_separators(QStringView& v) {
   for (size_t i = 0; i < v.size(); ++i) {
      if (!_is_directory_separator(v[i])) {
         v = v.mid(i);
         return;
      }
   }
   v = {};
}

// Check if the input string begins with the Data directory -- case-insensitive, 
// including a directory separator afterward. We don't check for any leading 
// directory separators, so be sure to strip those before calling this.
static bool _string_starts_with_data_dir(QStringView v) {
   if (v.size() < 5)
      return false;
   if (!_is_directory_separator(v[4]))
      return false;
   return v.startsWith(QLatin1String("data"), Qt::CaseInsensitive);
}

// Append the source string to the destination string, but fold redundant 
// directory separators, and convert all directory separators to backslashes. 
// The same check that folds redundant directory separators also causes us to 
// ignore leading directory separators in the source string.
static void _append_canonicalized_path_fragment(QString& dst, QStringView src) {
   bool has_non_separator_content = false;
   for (size_t i = 0; i < src.size(); ++i) {
      QChar c = src[i];
      if (_is_directory_separator(c)) {
         if (!has_non_separator_content)
            continue;
         has_non_separator_content = false;
         c = '\\';
      } else {
         has_non_separator_content = true;
      }
      dst += c;
   }
}

static void _deduplicate_directory_separators(QString& v) {
   uint size = v.size();
   for (uint i = 0; i < size; ++i) {
      QChar c = v[i];
      if (!_is_directory_separator(c))
         continue;
      uint extra_separator_count;
      {
         uint j = i + 1;
         for (; j < size; ++j) {
            if (!_is_directory_separator(v[j]))
               break;
         }
         extra_separator_count = j - i - 1;
      }
      if (extra_separator_count) {
         v.remove(i + 1, extra_separator_count);
         size -= extra_separator_count;
      }
   }
}

namespace ui::types {
   game_file_path::game_file_path(QStringView canonical_input_path) : game_file_path(canonical_input_path, canonical_options) {
   }
   game_file_path::game_file_path(QStringView v, const format_options& o) {
      _strip_leading_directory_separators(v);
      if (!o.include_data_directory) {
         this->_data = "Data\\";
      }
      this->_data += v.toString().replace('/', '\\');
      _deduplicate_directory_separators(this->_data);
   }

   QStringView game_file_path::data() const noexcept {
      return this->_data;
   }
   bool game_file_path::empty() const noexcept {
      return this->_data.isEmpty();
   }
   QString game_file_path::extension() const {
      if (this->empty())
         return {};
      QStringView view = this->_filename();
      if (view.isEmpty())
         return {};
      int i = view.lastIndexOf('.');
      if (i <= 0) // negative = not found; zero = ".htaccess" and whatnot
         return {};
      return view.mid(i).toString().toLower();
   }
   QString game_file_path::filename() const {
      return this->_filename().toString();
   }
   bool game_file_path::has_extension() const {
      auto view = this->_filename();
      int i = view.lastIndexOf('.');
      if (i <= 0)
         return false;
      return true;
   }
   bool game_file_path::has_filename() const {
      if (this->empty())
         return false;
      int i = this->_data.lastIndexOf('\\');
      if (i >= 0 && i < this->_data.size() - 1)
         return true;
      return false;
   }
   bool game_file_path::is_absolute() const noexcept {
      return this->_data.startsWith("Data\\", Qt::CaseInsensitive);
   }
   bool game_file_path::is_relative() const noexcept {
      return !this->empty() && !this->is_absolute();
   }

   game_file_path& game_file_path::append(QStringView content) {
      _strip_leading_directory_separators(content);
      if (content.isEmpty()) {
         if (this->_data.isEmpty())
            return *this;
         if (_is_directory_separator(this->_data.back()))
            return *this;
         //
         // If the current path doesn't end in a directory separator, then appending 
         // an empty segment should add a separator, i.e. "foo" + "" = "foo\\".
         //
         this->_data += '\\';
         return *this;
      }
      bool was_empty = this->_data.isEmpty();
      if (!was_empty && !_is_directory_separator(this->_data.back())) {
         this->_data += '\\';
      }
      this->_append_content(content);
      if (was_empty && !this->is_absolute()) {
         this->_data = QString("Data\\") + this->_data;
      }
      return *this;
   }
   game_file_path& game_file_path::append(const game_file_path& other) {
      if (other.is_absolute()) {
         *this = other;
         return *this;
      }
      return this->append(other._data);
   }
   game_file_path game_file_path::lexically_relative(game_file_path stem) const {
      if (this->empty() || stem.empty() || this->is_absolute() != stem.is_absolute())
         return {};
      if (!this->_data.startsWith(stem._data, Qt::CaseInsensitive))
         return {};
      game_file_path out;
      out._data = this->_data.mid(stem._data.size());
      return out;
   }
   game_file_path& game_file_path::make_absolute() {
      if (this->_data.isEmpty())
         return *this;
      auto view = QStringView(this->_data);
      if (view[0] == '/' || view[0] == '\\')
         view = view.mid(1);
      if (view.size() >= 5 && view.startsWith(QLatin1StringView("data", 4), Qt::CaseInsensitive)) {
         auto c = view[4];
         if (c == '/' || c == '\\')
            return *this;
      }
      this->_data = QString("Data\\") + view;
      return *this;
   }
   game_file_path& game_file_path::scope_to_stem_folder(std::string_view name) {
      auto name_view = QLatin1StringView(name.data(), name.size());

      int last = 0;
      do {
         int i = this->_data.indexOf(name_view, last, Qt::CaseInsensitive);
         if (i < 0) {
            break;
         }
         QChar prior = '\\';
         if (i > 0)
            prior = this->_data[i - 1];
         if (
            (prior == '/' || prior == '\\') &&
            i + name_view.size() < this->_data.size()
         ) {
            auto c = this->_data[i + name_view.size()];
            if (c.unicode() == '/' || c.unicode() == '\\') {
               this->_data = this->_data.mid(i);
               return *this;
            }
         }
         last = i + 1;
      } while (true);
      this->_data = QString(name_view) + "\\" + this->_data;
      _deduplicate_directory_separators(this->_data);
      return *this;
   }

   QString game_file_path::to_string(const format_options& o) const {
      if (this->empty())
         return {};
      QString     dst;
      QStringView src = this->_data;

      if (!o.include_data_directory && _string_starts_with_data_dir(src)) {
         src = src.mid(5);
      }
      dst = src.toString();
      if (!o.use_backslashes) {
         dst.replace('\\', '/');
      }
      return dst;
   }

   bool game_file_path::operator==(const game_file_path& other) const {
      return this->_data.compare(other._data, Qt::CaseInsensitive) == 0;
   }

   // Append the source string, but fold redundant directory separators, and 
   // convert all directory separators to the canonical separator. The same 
   // check that folds redundant directory separators also causes us to 
   // ignore leading directory separators in the source string.
   void game_file_path::_append_content(QStringView src) {
      bool has_non_separator_content = false;
      for (size_t i = 0; i < src.size(); ++i) {
         QChar c = src[i];
         if (_is_directory_separator(c)) {
            if (!has_non_separator_content)
               continue;
            has_non_separator_content = false;
            c = '\\';
         } else {
            has_non_separator_content = true;
         }
         this->_data += c;
      }
   }

   QStringView game_file_path::_filename() const {
      QStringView view = this->_data;
      int i = view.lastIndexOf('\\');
      if (i >= 0)
         view = view.mid(i + 1);
      return view;
   }
}