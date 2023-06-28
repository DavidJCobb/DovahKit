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
#include <version> // for feature test macros
#if __cpp_lib_unreachable >= 202202L
   #include <utility>
#endif

namespace cobb {
   #if __cpp_lib_unreachable >= 202202L
      #pragma push_macro("FORCEINLINE")
      #if defined(__GNUC__)
         #define FORCEINLINE __attribute__((always_inline))
      #elif defined(_MSC_VER)
         #define FORCEINLINE __forceinline
      #else
         #define FORCEINLINE
      #endif

      // const auto& unreachable = std::unreachable; // not viable; MSVC still emits warnings about control paths not returning values
      [[noreturn]] inline FORCEINLINE void unreachable() {
         if (std::is_constant_evaluated())
            throw;
         else
            std::unreachable();
      }

      #undef FORCEINLINE
      #pragma pop_macro("FORCEINLINE")
   #else
      #if defined(__GNUC__)
         [[noreturn]] inline __attribute__((always_inline)) void unreachable() {
            __builtin_unreachable();
            if (std::is_constant_evaluated())
               throw;
         }
      #elif defined(_MSC_VER)
         [[noreturn]] inline __forceinline void unreachable() {
            __assume(false);
            if (std::is_constant_evaluated())
               throw;
         }
      #else
         inline void unreachable() {
            if (std::is_constant_evaluated())
               throw;
         }
      #endif
   #endif
}