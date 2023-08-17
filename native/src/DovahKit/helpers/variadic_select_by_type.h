#include <type_traits>

namespace cobb {
   namespace impl::_variadic_select_by_type {
      template<typename Desired, typename T>
      constexpr Desired* _select_wrap(T& arg) {
         if constexpr (std::is_same_v<Desired, T>)
            return &arg;
         else
            return nullptr;
      }

      template<typename Desired, typename... Args>
      constexpr Desired* _select_ptr(Args&... args) {
         Desired* out = nullptr;
         (
            (out == nullptr ? out = _select_wrap<Desired, Args>(args) : nullptr),
            ...
         );
         return out;
      }
   }
   
   template<typename Desired, typename... Args>
   constexpr auto& variadic_select_by_type(Args&... args) requires (std::is_same_v<Desired, std::remove_const_t<Args>> || ...) {
      using OutType = std::conditional_t<
         std::is_const_v<Desired>,
         Desired,
         std::conditional_t<
            (std::is_same_v<Desired, Args> || ...),
            Desired,
            const Desired
         >
      >;
      return *impl::_variadic_select_by_type::_select_ptr<OutType, Args...>(std::forward<Args>(args)...);
   }
}