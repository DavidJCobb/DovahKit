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
#include "windows_ini.h"
#include "windows.h"
#include "intrusive_windows_defines.h"
#include "windows_environment.h"

namespace cobb {
   std::string read_single_ini_string_setting(const std::filesystem::path& file, const std::string& category, const std::string& key) {
      std::string fileA = file.string();
      std::string out;
      out.resize(256);
      DWORD count = GetPrivateProfileStringA(category.c_str(), key.c_str(), nullptr, out.data(), out.size(), fileA.c_str());
      while (count == out.size() - 1) {
         out.resize(out.size() + 256);
         count = GetPrivateProfileStringA(category.c_str(), key.c_str(), nullptr, out.data(), out.size(), fileA.c_str());
      }
      //
      auto end = out.find_last_not_of('\0');
      if (end != std::string::npos)
         out.resize(end + 1);
      //
      return out;
   }
   std::wstring read_single_ini_string_setting(const std::filesystem::path& file, const std::wstring& category, const std::wstring& key) {
      std::wstring fileW = file.wstring();
      std::wstring out;
      out.resize(256);
      DWORD count = GetPrivateProfileStringW(category.c_str(), key.c_str(), nullptr, out.data(), out.size(), fileW.c_str());
      while (count == out.size() - 1) {
         out.resize(out.size() + 256);
         count = GetPrivateProfileStringW(category.c_str(), key.c_str(), nullptr, out.data(), out.size(), fileW.c_str());
      }
      //
      auto end = out.find_last_not_of(L'\0');
      if (end != std::string::npos)
         out.resize(end + 1);
      //
      return out;
   }
}