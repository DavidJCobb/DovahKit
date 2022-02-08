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
#include <cmath>
#include <cstdint>

namespace nifDK {
   class file_reader;

   struct Float16 {
      static constexpr int fraction_bits = 10;
      static constexpr int exponent_bits =  5;

      static constexpr uint16_t fraction_mask = ((uint16_t(1) << fraction_bits) - 1);
      static constexpr uint16_t exponent_mask = ((uint16_t(1) << exponent_bits) - 1);

      static constexpr uint16_t infinity_pos = 0b0111110000000000;
      static constexpr uint16_t infinity_neg = 0b1111110000000000;
      static constexpr uint16_t nan_quiet    = 0b0111111111111111;
      static constexpr uint16_t nan_signal   = 0b1111111111111111;

      uint16_t value = 0;

      constexpr Float16() {}
      constexpr Float16(uint16_t d) : value(d) {}
      
      operator float() const;

      void read(file_reader&);
      void unchecked_read(file_reader&);
   };
}