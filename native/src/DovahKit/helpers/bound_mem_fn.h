#pragma once
#include <type_traits>

namespace cobb {
   //
   // A convenient way to generate a lambda that wraps an object's member function, 
   // such that `foo()` calls `self.foo_fn()`.
   //
   template<typename This, typename Return, typename... Args>
   constexpr auto bound_mem_fn(This& self, Return (This::* fn)(Args...)) {
      return [&self, fn](Args... args) -> Return { return (self.*fn)(std::forward<Args>(args)...); };
   }
}

// In any scope where `this` is defined, `cobb__bound_this_fn(member_name)` will 
// pass `this` and the named member function to `cobb::bound_mem_fn`.
#define cobb__bound_this_fn(fn_name) (cobb::bound_mem_fn(*this, &std::decay_t<decltype(*this)>::fn_name))