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
#include "combobox.h"
#include <QSortFilterProxyModel>

namespace cobb::qt {
   extern int map_combobox_index_from_proxy(QComboBox* combobox, int index) {
      auto* proxy = qobject_cast<QSortFilterProxyModel*>(combobox->model());
      if (!proxy)
         return index;
      auto* model = proxy->sourceModel();
      auto  p_qmi = proxy->index(index, 0);
      auto  s_qmi = proxy->mapToSource(p_qmi);
      if (!s_qmi.isValid())
         return -1;
      return s_qmi.row();
   }
   extern int map_combobox_index_to_proxy(QComboBox* combobox, int index) {
      auto* proxy = qobject_cast<QSortFilterProxyModel*>(combobox->model());
      if (!proxy)
         return index;
      auto* model = proxy->sourceModel();
      auto  s_qmi = model->index(index, 0);
      if (!s_qmi.isValid())
         return -1;
      auto  p_qmi = proxy->mapFromSource(s_qmi);
      return p_qmi.row();
   }
}