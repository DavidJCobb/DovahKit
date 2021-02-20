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
#include "windows_environment.h"
#include "windows.h"

namespace cobb::windows {
   void expand_environment_variables(std::string& original) {
      std::string out;
      out.resize(256);
      DWORD count = ExpandEnvironmentStringsA(original.c_str(), out.data(), out.size());
      while (count > out.size()) {
         out.resize(out.size() + 256);
         count = ExpandEnvironmentStringsA(original.c_str(), out.data(), out.size());
      }
      auto pos = out.find_last_not_of('\0');
      out.resize(pos == std::string::npos ? 0 : pos + 1);
      std::swap(original, out);
   }
   void expand_environment_variables(std::wstring& original) {
      std::wstring out;
      out.resize(256);
      DWORD count = ExpandEnvironmentStringsW(original.c_str(), out.data(), out.size());
      while (count > out.size()) {
         out.resize(out.size() + 256);
         count = ExpandEnvironmentStringsW(original.c_str(), out.data(), out.size());
      }
      auto pos = out.find_last_not_of(L'\0');
      out.resize(pos == std::wstring::npos ? 0 : pos + 1);
      std::swap(original, out);
   }
}