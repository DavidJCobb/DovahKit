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
   //
   // A ton of code uses multi-character literals like 'ABCD' to define fourCCs, but 
   // the behavior for these is implementation-defined. In practice, MSVC doesn't 
   // support eight-character literals, so we need a bit of a hack to make them work.
   //
   struct eight_cc {
      private:
         /*not constexpr!*/ uint64_t do_not_use_more_than_eight_chars() { return 0; }
      public:
         union {
            uint64_t value = 0;
            uint32_t halves[2]; // for easier viewing in VS's debugger
            char     bytes[8];
         };
         //
         eight_cc() {}
         constexpr eight_cc(const char* s) {
            this->value = 0;
            if (!s)
               return;
            uint8_t i = 0;
            for (; i < 8; ++i) {
               if (!s[i])
                  break;
               this->value |= uint64_t(s[i]) << (0x08 * i);
            }
            if (i == 8 && s[8]) // don't allow inputs longer than eight characters
               //
               // (static_assert) can't be given variables even if it's called inside of a 
               // constexpr function, so instead, we just have to call a non-constexpr 
               // function that does nothing (and so will hopefully be optimized out) and 
               // whose name should clue a programmer into the problem.
               // 
               this->value += do_not_use_more_than_eight_chars();
         }
         //
         inline constexpr eight_cc& operator=(const eight_cc& other) noexcept {
            this->value = other.value;
            return *this;
         }
         //
         inline constexpr operator uint64_t() const noexcept { return this->value; }
   };
}