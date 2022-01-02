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
#include <initializer_list>
#include <utility>

namespace cobb {
   namespace impl {
      template<auto Value, size_t Count, typename = std::make_index_sequence<Count>> struct n_of_value;
      template<auto Value, size_t Count, size_t... Indices> struct n_of_value<Value, Count, std::index_sequence<Indices...>> {
            static constexpr std::initializer_list<decltype(Value)> list = { (Indices, Value)... };
      };
   }

   // Generate an initializer_list consisting of (Count) repetitions of (Value). Note that as of 
   // January 2022, Visual Studio 2019 IntelliSense does not show valid or even sensible tooltips 
   // for these values; expect nonsense like:
   //
   //    auto test = cobb::n_of_value<1, 3>; // tooltip: {&{1,1,1}, {1,1,1} + 3}
   // 
   // I've tried writing the template in several different ways. It's not the template, but rather 
   // the generated value itself, that IntelliSense is choking on. My apologies.
   // 
   // Separately from that, this template is of limited utility because brace initialization isn't 
   // the same as initializing from an std::initializer_list; the latter is an almost uselessly 
   // clumsy mimicry of the former. You can't, for example, use cobb::n_of_value to initialize a 
   // std::array (use cobb::array_of_n_values instead).
   //
   template<auto Value, size_t Count> static constexpr std::initializer_list<decltype(Value)> n_of_value = impl::n_of_value<Value, Count>::list;
}