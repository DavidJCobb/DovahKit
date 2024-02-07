#pragma once
#include <concepts>
#include <string>
#include <vector>

namespace dovah::pex::parsers::util {
   template<typename T, typename Stream>
   concept supports_stream = requires(T& v, Stream& s) {
      { v.read(s) };
      { T::skip(s) };
   };

   template<typename Stream>
   concept is_valid_stream_class = requires(Stream& stream, std::string& s, std::vector<int>& v, int& i) {
      { stream.get_position() } -> std::same_as<size_t>;

      // single-read support
      { stream.read(i) };
      { stream.template read_length_prefixed_string<1>(s) };
      { stream.template read_length_prefixed_vector<1>(v) };

      { stream.read(i, v) }; // multi-read support

      // skip support
      { stream.skip_bytes(1) };
      { stream.template skip_length_prefixed_string<1>() };
      { stream.template skip_length_prefixed_vector<1, int>() };

      // string-table lookup support
      { stream.get_tabled_string(1) } -> std::same_as<const std::string&>;
   };
}
