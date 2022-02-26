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
#include <string>

namespace cobb {
   namespace windows_registry {
      #if UNICODE
         using char_t    = wchar_t;
         using string_t  = std::wstring;
      #else
         using char_t    = char;
         using string_t  = std::string;
      #endif
      using cstring_t = const char_t*;

      enum class hkey {
         classes_root,
         current_config,
         current_user,
         local_machine,
         performance_data,
         performance_nlstext,
         performance_text,
         users,
      };
      //
      bool get_string_value(hkey, cstring_t key, cstring_t value, string_t& out);
   }
}