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
#include <string>

namespace cobb {
   template<char... c> struct static_string {
      const char s[sizeof...(c)] = { c... };

      consteval size_t size() const noexcept { return sizeof...(c); }
      consteval const char* data() const noexcept { return s; }
      consteval const std::string string() const noexcept { return s; }
   };

   template<uint32_t sig> using string_from_four_cc = static_string<
      (char)(sig >> 0x18),
      (char)(sig >> 0x10),
      (char)(sig >> 0x08),
      (char)sig,
      0
   >;
}