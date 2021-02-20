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
#include <filesystem>

namespace cobb {
   enum class filename_validation_result {
      valid,
      missing,
      is_a_path,
      is_current_or_parent_directory, // "." or ".."
      windows_device_name, // CON, NUL, etc.
      illegal_character,
      ends_in_period,
   };

   extern filename_validation_result validate_filename(const std::filesystem::path& filename, bool require_stem = true);

   extern bool filename_has_extension(const std::filesystem::path& filename, const std::initializer_list<const char*> extensions); // case-insensitive, but extensions must be representable in latin-1
}