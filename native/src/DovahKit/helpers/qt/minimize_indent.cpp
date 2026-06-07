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
#include "minimize_indent.h"

namespace cobb::qt {
   extern uint get_minimum_indent(const QString& text, uint* out_lns) {
      if (text.isEmpty()) {
         if (out_lns)
            *out_lns = 0;
         return 0;
      }
      uint lowest  = std::numeric_limits<uint>::max();
      uint current = 0;
      //
      uint last_non_space = text.size() - 1;
      for (uint i = 0; i < text.size(); ++i) {
         QChar c = text[i];
         if (c == '\r')
            continue;
         if (c == '\n') {
            current = 0;
            continue;
         }
         if (!c.isSpace()) {
            last_non_space = i;
            if (current < lowest)
               lowest = current;
            continue;
         }
         ++current;
      }
      if (out_lns)
         *out_lns = last_non_space;
      return lowest;
   }
   extern QString minimize_indent(const QString& text) {
      QString out;
      if (text.isEmpty())
         return out;
      uint last_non_space;
      uint lowest  = get_minimum_indent(text, &last_non_space);
      uint current = 0;
      bool is_line_start = true;
      for (uint i = 0; i <= last_non_space; ++i) {
         QChar c = text[i];
         if (c == '\r' || c == '\n' || !c.isSpace() || !is_line_start) {
            out += c;
            current = 0;
            if (!c.isSpace())
               is_line_start = false;
            else if (c == '\r' || c == '\n')
               is_line_start = true;
            continue;
         }
         ++current;
         if (current > lowest)
            out += c;
      }
      //
      // Removing leading and trailing line breaks:
      //
      uint size  = out.size();
      uint start = 0;
      uint end   = size;
      for (uint i = 0; i < size; ++i) {
         QChar c = out[i];
         if (c == '\r' || c == '\n')
            ++start;
         else
            break;
      }
      for (uint i = size - 1; i >= start; ++i) {
         QChar c = out[i];
         if (c == '\r' || c == '\n')
            --end;
         else
            break;
      }
      if (end < size)
         out.chop(size - end);
      if (start > 0)
         out = out.mid(start);
      //
      return out;
   }
}