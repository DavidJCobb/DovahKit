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
#include "strings.h"

#include "../windows.h"

namespace cobb::qt {
   QString four_cc_to_string(uint32_t signature) {
      return QString("%1%2%3%4")
         .arg(QChar(signature >> 0x18))
         .arg(QChar((signature >> 0x10) & 0xFF))
         .arg(QChar((signature >> 0x08) & 0xFF))
         .arg(QChar(signature & 0xFF));
   }
   QString winapi_code_to_string(uint32_t code) {
      QString win_text;
      //
      void* message;
      uint32_t size = FormatMessage(
         FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
         nullptr,
         code,
         LANG_USER_DEFAULT,
         (LPTSTR)&message,
         0,
         nullptr
      );
      uint32_t i = 0;
      while (wchar_t c = ((const wchar_t*)message)[i++])
         win_text += c;
      LocalFree(message);
      //
      return win_text;
   }
}