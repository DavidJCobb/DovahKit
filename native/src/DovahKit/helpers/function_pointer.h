#pragma once

namespace cobb {
   namespace impl {
      template<typename> struct _function_pointer;
      template<typename Return, typename... Args> struct _function_pointer<Return(Args...)> {
         using type = Return(*)(Args...);
      };
   }

   //
   // A less cursed syntax for function pointers. This uses the same syntax as std::function 
   // but is non-polymorphic: it produces a bare function pointer, and does not support the 
   // use of lambdas or other structs that implement operator().
   //
   template<typename T> using function_pointer = impl::_function_pointer<T>::type;
}