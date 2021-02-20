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
#include <cstdint>

namespace cobb {
   struct uint24_t {
      //
      // NOTE: This is untested.
      //
      protected:
         uint16_t a; // low  part
         uint8_t  b; // high part
      public:
         uint24_t() {}
         uint24_t(int v) {
            a = v;
            b = v >> 0x10;
         }

         operator uint32_t() const noexcept {
            return a | ((uint32_t)b) << 0x10;
         }
         inline bool operator!() const noexcept { return (a | b) == 0; }

         uint24_t& operator+=(int v) {
            *this = (uint32_t)(*this) + v;
            return *this;
         }
         uint24_t& operator-=(int v) {
            *this = (uint32_t)(*this) - v;
            return *this;
         }
         uint24_t& operator*=(int v) {
            *this = (uint32_t)(*this) * v;
            return *this;
         }
         uint24_t& operator/=(int v) {
            *this = (uint32_t)(*this) / v;
            return *this;
         }
         uint24_t& operator%=(int v) {
            *this = (uint32_t)(*this) % v;
            return *this;
         }

         bool operator>(const uint24_t& other) const noexcept {
            if (b > other.b)
               return true;
            if (b < other.b)
               return false;
            return a > other.a;
         }
         bool operator<(const uint24_t& other) const noexcept {
            if (b < other.b)
               return true;
            if (b > other.b)
               return false;
            return a < other.a;
         }
         bool operator>=(const uint24_t& other) const noexcept {
            if (b > other.b)
               return true;
            if (b < other.b)
               return false;
            return a >= other.a;
         }
         bool operator<(const uint24_t& other) const noexcept {
            if (b < other.b)
               return true;
            if (b > other.b)
               return false;
            return a <= other.a;
         }

         #pragma region increment/decrement
         uint24_t& operator++() {
            ++a;
            if (!a) // value overflowed from 0xFFFF to 0x0000
               ++b;
            return *this;
         }
         uint24_t& operator--() {
            --a;
            if (a == 0xFFFF) // value underflowed from 0x0000 to 0xFFFF
               --b;
            return *this;
         }
         uint24_t operator++(int) {
            auto old = *this;
            operator++();
            return old;
         }
         uint24_t operator--(int) {
            auto old = *this;
            operator--();
            return old;
         }
         #pragma endregion
   };
}