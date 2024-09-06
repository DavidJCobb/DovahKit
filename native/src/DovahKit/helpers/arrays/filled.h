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
#include <array>
#include <utility>

namespace cobb::arrays {
   namespace impl {
      template<std::size_t Size, typename ValueType, typename ArgType, std::size_t... Indices>
      constexpr auto filled(ArgType&& value, std::index_sequence<Indices...>) {
         return std::array<std::decay_t<ValueType>, Size>{ (static_cast<void>(Indices), ValueType{value})..., ValueType{std::forward<ArgType>(value)} };
      }
   }

   //
   // Create an array filled with the same value.
   //
   template<size_t Size, typename ValueType, typename ArgType = ValueType>
   constexpr std::array<ValueType, Size> filled(ArgType&& value) {
      return impl::filled<Size, ValueType, ArgType>(std::forward<ArgType>(value), std::make_index_sequence<Size - 1>{});
   }
}