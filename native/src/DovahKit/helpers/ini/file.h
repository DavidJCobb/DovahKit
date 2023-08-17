#pragma once
#include <istream>
#include <ostream>
#include <string_view>
#include <vector>
#include "../passkey.h"

namespace cobb::ini {
   class category;

   class file {
      protected:
         // std::istringstream and similar aren't constexpr, so we need to roll our own 
         // stream-style string wrappers in order to be able to constexpr-load/save a 
         // std::string in compile-time tests.
         #pragma region _input_stream
            template<typename T> class _input_stream;

            template<> class _input_stream<std::istream> {
               private:
                  std::istream& dst;

               public:
                  _input_stream(std::istream& d) : dst(d) {}

                  inline bool operator!() const { return !dst; }
                  explicit inline operator bool() const { return (bool)dst; }

                  inline bool has_more() const { return !dst.bad() && !dst.eof(); }
                  inline void get_next_line(std::string& out) { std::getline(dst, out); }
            };

            template<> class _input_stream<std::string> {
               private:
                  const std::string& src;
                  size_t pos = 0;

               public:
                  constexpr _input_stream(const std::string& d) : src(d) {}

                  constexpr bool operator!() const { return false; } // check if error occurred
                  explicit constexpr operator bool() const { return true; } // check if no error occurred

                  constexpr bool has_more() const;
                  constexpr void get_next_line(std::string& out);
            };
         #pragma endregion
         #pragma region _output_stream
            template<typename T> class _output_stream;

            template<> class _output_stream<std::ostream> {
               private:
                  std::ostream& dst;

               public:
                  _output_stream(std::ostream& d) : dst(d) {}

                  _output_stream& operator<<(char c) { dst << c; return *this; }
                  _output_stream& operator<<(const char* s) { dst << s; return *this; }
                  _output_stream& operator<<(const std::string& s) { dst << s; return *this; }

                  _output_stream& put(char c) { dst.put(c); return *this; }
            };

            template<> class _output_stream<std::string> {
               private:
                  std::string& dst;

               public:
                  constexpr _output_stream(std::string& d) : dst(d) {}

                  constexpr _output_stream& operator<<(char c) { dst += c; return *this; }
                  constexpr _output_stream& operator<<(const char* s) { dst += s; return *this; }
                  constexpr _output_stream& operator<<(const std::string& s) { dst += s; return *this; }

                  constexpr _output_stream& put(char c) {
                     if (c == '\n')
                        dst += '\r';
                     dst += c;
                     return *this;
                  }
            };
         #pragma endregion

      protected:
         std::vector<category*> _categories;

         template<typename T>
         static void _write_category(_output_stream<T>& dst, const category&);

      public:
         constexpr const std::vector<category*>& categories() const noexcept {
            return this->_categories;
         }

         constexpr category* category_by_name(std::string_view name) const;

      protected:
         template<typename T> constexpr void _load(_input_stream<T>);
         template<typename T> constexpr void _save(_output_stream<T> dst);
         template<typename OutType, typename InType> constexpr void _save(_output_stream<OutType> dst, _input_stream<InType> src);

      public:
         constexpr void load(const std::string&);
         constexpr void save(std::string& dst);
         constexpr void save(std::string& dst, const std::string& src);

         void load(std::istream&);
         void save(std::ostream& dst);
         void save(std::ostream& dst, std::istream& src); // preserves the existing file's whitespace, comments, setting order, etc.; writes all setting values including those not changed from the defaults

         constexpr void _on_category_instantiated(::cobb::passkey<file, category>, category&);
   };
}

#include "./file.inl"