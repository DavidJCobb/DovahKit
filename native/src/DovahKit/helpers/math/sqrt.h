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
#include <limits>

namespace cobb {
   namespace ct {
      double constexpr sqrt(double v) {
         if (v < 0 || v >= std::numeric_limits<double>::infinity())
            return std::numeric_limits<double>::quiet_NaN();
         //
         // Use the Newton-Raphson method:
         //
         double current  = v;
         double previous = 0;
         while (current != previous) {
            previous = current;
            current  = 0.5 * (current + (v / current));
         }
         return current;
      }
   }
   
   constexpr double sqrt(double v) noexcept {
      if (std::is_constant_evaluated()) {
         return ct::sqrt(v);
      } else {
         return std::sqrt(v);
      }
   }
}