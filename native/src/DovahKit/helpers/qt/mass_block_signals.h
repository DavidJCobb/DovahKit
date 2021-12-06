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
#include <array>
#include <type_traits>
#include <QObject>
#include <QSignalBlocker>
#include "../concepts.h"

namespace cobb::qt {
   template<typename... Types> requires cobb::is_base_of_all<QObject, std::remove_pointer_t<Types>...>
   struct mass_signal_blocker {
      protected:
         std::array<QSignalBlocker, sizeof...(Types)> blockers;
      public:
         mass_signal_blocker(Types... t) : blockers({ QSignalBlocker(t)... }) {}
   };
}