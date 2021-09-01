/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#pragma once
#include <functional>
#include <type_traits>

namespace cobb {

   // Templates to inspect the value of any non-overloaded function.
   //
   // When using function_traits, the template argument must be decltype(&function), while when using the 
   // individual-trait templates, the template argument must be &function. IntelliSense will accept a 
   // direct reference to the function, but MSVC itself will likely encounter an internal compiler error 
   // that it does not know how to report.

   template<typename T> struct function_traits {};
   template<typename R, typename ...Args> struct function_traits<std::function<R(Args...)>> {
      static constexpr const size_t arg_count = sizeof...(Args);
      static constexpr bool is_const_function = false;
      using return_type  = R;
      using context_type = void;

      template<size_t i> using arg_type = typename std::tuple_element<i, std::tuple<Args...>>::type;
   };
   template<typename R, typename ...Args> struct function_traits<R(*)(Args...)> {
      static constexpr const size_t arg_count = sizeof...(Args);
      static constexpr bool is_const_function = false;
      using return_type  = R;
      using context_type = void;

      template<size_t i> using arg_type = typename std::tuple_element<i, std::tuple<Args...>>::type;
   };
   template<typename T, typename R, typename ...Args> struct function_traits<R(T::*)(Args...)> {
      static constexpr const size_t arg_count = sizeof...(Args);
      static constexpr bool is_const_function = false;
      using return_type  = R;
      using context_type = T;

      template<size_t i> using arg_type = typename std::tuple_element<i, std::tuple<Args...>>::type;
   };
   template<typename T, typename R, typename ...Args> struct function_traits<R(T::*)(Args...) const> {
      static constexpr const size_t arg_count = sizeof...(Args);
      static constexpr bool is_const_function = true;
      using return_type  = R;
      using context_type = T;

      template<size_t i> using arg_type = typename std::tuple_element<i, std::tuple<Args...>>::type;
   };

   template<auto func, size_t n> using type_of_nth_argument = function_traits<decltype(func)>::template arg_type<n>;
   template<auto func> using return_type_of = function_traits<decltype(func)>::return_type;
   template<auto func> using member_function_context_type = function_traits<decltype(func)>::context_type;
   template<auto func> static constexpr bool is_const_function = function_traits<decltype(func)>::is_const_function;
}
