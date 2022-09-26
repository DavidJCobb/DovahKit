#pragma once
#include <array>
#include <concepts>
#include <type_traits>
#include "./class_array.h"

namespace cobb {
   namespace impl::_class_map {
      template<typename Functor, typename Class, typename ValueType, typename... Args>
      concept can_templated_invoke = requires(Functor&& f, ValueType& v, Args&&... args) {
         { f.template operator()<Class>(v, std::forward<Args>(args)...) };
      };

      template<typename Result, typename Functor, typename Class, typename ValueType, typename... Args>
      concept can_templated_invoke_with_result = requires(Functor&& f, ValueType& v, Args&&... args) {
         { f.template operator()<Class>(v, std::forward<Args>(args)...) } -> std::same_as<Result>;
      };
   }

   template<typename ValueType, typename... Classes>
   class class_map {
      public:
         using classes    = cobb::class_array<Classes...>;
         using value_type = ValueType;

      protected:
         std::array<value_type, sizeof...(Classes)> _values = {};

      public:
         template<typename C> requires (std::is_same_v<Classes, C> || ...)
         constexpr value_type& value_for() noexcept { return this->_values[classes::template index_of_type<C>]; }
         template<typename C> requires (std::is_same_v<Classes, C> || ...)
         constexpr const value_type& value_for() const noexcept { return this->_values[classes::template index_of_type<C>]; }
         
         template<typename Functor, typename... Args>
         constexpr void for_each(Functor&& f, Args&&... args) {
            if constexpr ((impl::_class_map::can_templated_invoke<Functor, Classes, value_type&, Args...> && ...)) {
               (f.template operator()<Classes>(this->value_for<Classes>(), std::forward<Args>(args)...), ...);
            } else {
               (f(this->value_for<Classes>(), std::forward<Args>(args)...), ...);
            }
         }
         template<typename Functor, typename... Args>
         constexpr void for_each(Functor&& f, Args&&... args) const {
            if constexpr ((impl::_class_map::can_templated_invoke<Functor, Classes, std::add_const_t<value_type>&, Args...> && ...)) {
               (f.template operator()<Classes>(this->value_for<Classes>(), std::forward<Args>(args)...), ...);
            } else {
               (f(this->value_for<Classes>(), std::forward<Args>(args)...), ...);
            }
         }

         template<typename Functor, typename... Args>
         constexpr bool for_each_until_false(Functor&& f, Args&&... args) {
            if constexpr ((impl::_class_map::can_templated_invoke_with_result<bool, Functor, Classes, value_type&, Args...> && ...)) {
               (f.template operator()<Classes>(this->value_for<Classes>(), std::forward<Args>(args)...) || ...);
            } else {
               (f(this->value_for<Classes>(), std::forward<Args>(args)...) || ...);
            }
         }
         template<typename Functor, typename... Args>
         constexpr bool for_each_until_false(Functor&& f, Args&&... args) const {
            if constexpr ((impl::_class_map::can_templated_invoke_with_result<bool, Functor, Classes, std::add_const_t<value_type>&, Args...> && ...)) {
               (f.template operator()<Classes>(this->value_for<Classes>(), std::forward<Args>(args)...) || ...);
            } else {
               (f(this->value_for<Classes>(), std::forward<Args>(args)...) || ...);
            }
         }
   };

   namespace impl::_class_map {
      template<typename ValueType, typename ClassArray> struct from_class_array;
      template<typename ValueType, typename... Classes> struct from_class_array<ValueType, cobb::class_array<Classes...>> {
         using type = class_map<ValueType, Classes...>;
      };
   }

   template<typename ValueType, typename ClassArray> using class_map_from_class_array = typename impl::_class_map::from_class_array<ValueType, ClassArray>::type;
}
