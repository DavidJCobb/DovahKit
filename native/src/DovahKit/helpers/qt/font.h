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
#include <QFont>

namespace cobb::qt {

   //
   // A QFont is a set of font properties, as well as an internal flags-mask indicating which 
   // properties have actually been set. There are cases where you may wish to clear properties 
   // from a QFont or query which properties are actually set, but there are no documented 
   // functions for doing so. That doesn't mean that it's impossible; it just means that it may 
   // become impossible in the future...
   //

   extern void clear_font_properties(QFont&, uint mask) noexcept;
   extern bool test_font_properties(const QFont&, uint mask) noexcept; // use QFont::ResolveProperty for mask bits
}