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
#include "font.h"

namespace cobb::qt {
   extern uint get_font_property_presence_mask(const QFont& font) noexcept {
      uint mask;
      #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
         mask = font.resolve();
      #else
         mask = font.resolveMask();
      #endif
      return mask;
   }
   extern void clear_font_properties(QFont& font, uint mask_to_clear) noexcept {
      uint mask = get_font_property_presence_mask(font);
      mask &= ~mask_to_clear;
      #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
         font.resolve(mask);
      #else
         font.setResolveMask(mask);
      #endif
      //
      // Detach the font's shared state:
      //
      font = font.resolve(QFont());
   }
   extern bool test_font_properties(const QFont& font, uint mask_to_test) noexcept {
      uint mask = get_font_property_presence_mask(font);
      return (mask & mask_to_test) == mask_to_test;
   }
}