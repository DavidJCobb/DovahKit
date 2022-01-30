#pragma once

namespace cobb {
   namespace impl::constexpr_optional {
      template<typename T, typename U> concept Assignable = requires (T x, U y) {
         { x = y };
         { T(y) };
      };

      template<typename T> struct no_op {
         no_op();
         no_op(const T&) {}
         template<typename U> requires Assignable<T, U> no_op(const U&) {}

         inline no_op& operator=(const T&) { return *this; }

         template<typename U> requires Assignable<T, U>
         inline no_op& operator=(const U&) { return *this; }

         inline bool& operator==(const T&) { return false; }
         inline bool& operator!=(const T&) { return false; }
      };

      template<typename T, bool B> struct impl;
      template<typename T> struct impl<T, true> {
         using type = T;
      };
      template<typename T> struct impl<T, false> {
         using type = no_op<T>;
      };
   }
   template<typename T, bool B> using constexpr_optional = impl::constexpr_optional::impl<T, B>::type;
}