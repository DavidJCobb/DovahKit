#pragma once
#include <array>
#include <limits>
#include <string_view>
#include "../common_lambdas.h"
#include "../property_definition.h"
#include "../utils/is_accessor_for_type.h"

namespace dovahscript::api_helpers::subobject_property_helpers {
   namespace impl {
      namespace integer_property_range_error {
         constexpr const std::string_view text_leading  = "value must be within the range [";
         constexpr const std::string_view text_middle   = ", ";
         constexpr const std::string_view text_trailing = "]";

         template<intmax_t V>
         constexpr const size_t text_length_for_integer = []() consteval -> size_t {
            if (V == 0)
               return 1;
            size_t i = 0;
            auto   v = V;
            if (v < 0) {
               ++i;
               v = -v;
            }
            while (v) {
               ++i;
               v = v / 10;
            }
            return i;
         }();

         template<auto Min, auto Max>
         constexpr const size_t text_length =
            text_leading.size() +
            text_middle.size() +
            text_trailing.size() +
            text_length_for_integer<Min> +
            text_length_for_integer<Max> +
            sizeof('\0');
         
         template<auto Min, auto Max>
         constexpr const auto buffer = []() consteval {
            std::array<char, text_length<Min, Max>> buffer = { 0 };

            size_t i = 0;
            auto print_text = [&i, &buffer](std::string_view text) {
               std::copy(text.begin(), text.end(), buffer.begin() + i);
               i += text.size();
            };
            auto print_int = [&i, &buffer](intmax_t v) {
               if (v == 0) {
                  buffer[i++] = '0';
                  return;
               }

               if (v < 0) {
                  buffer[i++] = '-';
                  v = -v;
               }
               int digit_count = 0;
               while (v) {
                  auto digit = v % 10;
                  v /= 10;

                  buffer[i] = (char)('0' + digit);
                  ++i;
                  ++digit_count;
               }
               if (digit_count > 1) {
                  std::reverse(buffer.begin() + i - digit_count, buffer.begin() + i);
               }
            };
            print_text("value must be within the range [");
            print_int(Min);
            print_text(", ");
            print_int(Max);
            print_text("]");

            return buffer;
         }();

         template<auto Min, auto Max>
         constexpr const auto string_view = std::string_view(buffer<Min, Max>.data());
      }
   }

   template<
      auto Minimum = std::numeric_limits<lua_Integer>::lowest(),
      auto Maximum = std::numeric_limits<lua_Integer>::max()
   >
   struct integer_property {
      integer_property() = delete;

      static std::string_view check_lua_value(lua_State* L, int pos) {
         if (!lua_isinteger(L, pos))
            return "integer expected";
         auto v = lua_tointeger(L, pos);
         if constexpr (
            Minimum > std::numeric_limits<lua_Integer>::lowest() &&
            Maximum < std::numeric_limits<lua_Integer>::max()
         ) {
            if (v < Minimum || v > Maximum) {
               return impl::integer_property_range_error::string_view<Minimum, Maximum>;
            }
         }
         return {};
      }

      template<typename T>
      static void push_lua_value(lua_State* L, const T& v) {
         lua_pushinteger(L, v);
      }
      
      template<typename AccessFunc>
      static consteval auto define(std::string_view name, AccessFunc a) {
         return property_definition{
            .name   = name,
            .access = a,
            .check  = &check_lua_value,
            .pull   = &pull::integer,
            .push   = &push_lua_value<std::decay_t<typename cobb::function_traits<AccessFunc>::return_type>>,
         };
      }

      template<typename AccessFunc>
      static consteval auto define(std::string_view name, AccessFunc a, lua_Integer dv) {
         return property_definition{
            .name   = name,
            .access = a,
            .check  = &check_lua_value,
            .pull   = &pull::integer,
            .push   = &push_lua_value<std::decay_t<typename cobb::function_traits<AccessFunc>::return_type>>,
            .default_value = dv,
         };
      }
   };
}