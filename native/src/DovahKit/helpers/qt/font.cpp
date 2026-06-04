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

   //
   // These functions rely on undocumented, but public, functions on QFont.
   // 
   // UPDATE: These functions are unavailable as of Qt 6; we now use vile hacks to 
   // break into QFont's private state.
   //

   extern void clear_font_properties(QFont& font, uint mask) noexcept {
      //
      // Let's start by modifying the mask.
      //
      *(uint*)((uint8_t*)&font) &= ~(uint)mask;
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
      auto resolve_mask = *(const uint*)((const uint8_t*)&font + 8);
      return (resolve_mask & mask) == mask;
   }
}