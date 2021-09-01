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

namespace {
   static_assert(std::is_same_v<decltype(std::declval<QFont&>().resolve()),  uint>, "This code relied on a QFont::resolve overload that was neither deprecated nor documented. Said overload returned the font's internal mask of resolved properties.");
   static_assert(std::is_same_v<decltype(std::declval<QFont&>().resolve(0)), void>, "This code relied on a QFont::resolve overload that was neither deprecated nor documented. Said overload modified the font's internal mask of resolved properties.");
}
namespace cobb::qt {

   //
   // These functions rely on undocumented, but public, functions on QFont.
   //

   extern void clear_font_properties(QFont& font, uint mask) noexcept {
      //
      // Let's start by modifying the mask.
      //
      auto resolved = font.resolve();
      font.resolve(resolved &= ~(uint)mask);
      //
      // Modifying the mask isn't enough; QFont has some internal state that we can't access or 
      // reset from the outside. Fortunately, there's a way to work around this by replacing 
      // the font with a modified copy of itself.
      // 
      // If we call {QFont QFont::resolve(const QFont&) const} on our font, the function will 
      // create and return a new QFont, copying fields as appropriate without copying whatever 
      // it is that makes merely clearing the flags insufficient.
      //
      font = font.resolve(QFont());
   }
   extern bool test_font_properties(const QFont& font, uint mask) noexcept {
      return (font.resolve() & mask) == mask;
   }
}