#pragma once
#include <QString>

namespace ui::types {
   class game_file_path {
      public:
         struct format_options {
            bool include_data_directory = true;
            bool use_backslashes        = true;
         };

         static constexpr const format_options canonical_options = {\
            .include_data_directory = true,
            .use_backslashes        = true,
         };

      protected:
         QString _data; // stored in canonical form

      public:
         game_file_path() {}
         game_file_path(const char* canonical_input_path) : game_file_path(QString(canonical_input_path)) {};
         game_file_path(QStringView canonical_input_path);
         game_file_path(QStringView input_path, const format_options& input_path_format);
         game_file_path(const char* input_path, const format_options& input_path_format) : game_file_path(QString(input_path), input_path_format) {};

         QStringView data() const noexcept;
         bool empty() const noexcept;
         QString extension() const;
         QString filename() const;
         bool has_extension() const;
         bool has_filename() const;
         bool is_absolute() const noexcept; // true if the path is non-empty and starts with case-insensitive "Data\"
         bool is_relative() const noexcept; // true if the path is non-empty and doesn't start with case-insensitive "Data\"

         game_file_path& append(QStringView);
         game_file_path& append(const game_file_path&);
         game_file_path lexically_relative(game_file_path) const;
         game_file_path& make_absolute(); // prepends Data-directory prefix if it's absent
         game_file_path& scope_to_stem_folder(std::string_view folder_name); // mimics the game's behavior, except that the argument you pass should not include a directory separator

         QString to_string(const format_options& fmt = canonical_options) const;

         bool operator==(const game_file_path&) const; // case-insensitive

      protected:
         void _append_content(QStringView);
         QStringView _filename() const;
   };
}