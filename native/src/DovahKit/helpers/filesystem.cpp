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
#include "filesystem.h"
#include <array>

namespace cobb {
   namespace {
      std::array<const char*, 23> _windows_devices = {{ // word is you can use these friggin' things if you prefix the full path with \\?\ (that is, "\\\\?\\") but i'm not gonna bother lol
         "aux",
         "com0",
         "com1",
         "com2",
         "com3",
         "com4",
         "com5",
         "com6",
         "com7",
         "com8",
         "com9",
         "con",
         "lpt1",
         "lpt2",
         "lpt3",
         "lpt4",
         "lpt5",
         "lpt6",
         "lpt7",
         "lpt8",
         "lpt9",
         "nul",
         "prn",
      }};
      static const std::string _illegal_chars = "<>:\"|?*";
   }
   filename_validation_result validate_filename(const std::filesystem::path& filename, bool require_stem) {
      using _result = filename_validation_result;
      //
      #if _DEBUG
         auto parent = filename.parent_path();
      #endif
      if (filename.has_root_path() || filename.has_parent_path())
         return _result::is_a_path;
      if (require_stem) {
         if (!filename.has_stem() || filename.stem().c_str()[0] == '.')
            return _result::missing;
      } else {
         if (filename.empty())
            return _result::missing;
      }
      auto name = filename.stem(); // does not include extension
      if (name == "." || name == "..")
         return _result::is_current_or_parent_directory;
      auto nam8 = name.string(); // single-byte string; useful for comparing Latin-1 strings such as Windows device names
      for (auto* device : _windows_devices)
         if (_stricmp(nam8.data(), device) == 0)
            return _result::windows_device_name;
      //
      // <https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file>
      for (auto c : name.wstring()) {
         if (c < 32)
            return _result::illegal_character;
         for (auto d : _illegal_chars)
            if (c == d)
               return _result::illegal_character;
      }
      if (filename.has_extension()) {
         if (filename.extension().string().back() == '.')
            return _result::ends_in_period;
      }
      //
      return _result::valid;
   }

   bool filename_has_extension(const std::filesystem::path& filename, const std::initializer_list<const char*> extensions) {
      if (!filename.has_extension())
         return false;
      auto ext = filename.extension().string();
      for (auto* e : extensions) {
         if (_stricmp(ext.data(), e) == 0)
            return true;
      }
      return false;
   }
}