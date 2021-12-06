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
#include <type_traits>
#include <QComboBox>

namespace cobb::qt {
   extern int map_combobox_index_from_proxy(QComboBox* combobox, int index);
   extern int map_combobox_index_to_proxy(QComboBox* combobox, int index);

   template<typename T> requires ((std::is_enum_v<std::decay_t<T>> && sizeof(std::underlying_type_t<T>) <= sizeof(int)) || std::is_arithmetic_v<T>)
   void set_combobox_value(QComboBox* combobox, T value) {
      if constexpr (std::is_enum_v<std::decay_t<T>>) {
         auto i = combobox->findData((int)value);
         if (i >= 0)
            combobox->setCurrentIndex(i);
      } else {
         auto i = combobox->findData(value);
         if (i >= 0)
            combobox->setCurrentIndex(i);
      }
   }
}