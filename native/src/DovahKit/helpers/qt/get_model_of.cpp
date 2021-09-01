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
#include "get_model_of.h"
#include <QAbstractItemView>
#include <QComboBox>
#include <QSortFilterProxyModel>

namespace cobb::qt {
   extern QAbstractItemModel* get_model_of(QWidget* root) {
      if (auto* casted = qobject_cast<QAbstractItemView*>(root))
         return casted->model();
      if (auto* casted = qobject_cast<QComboBox*>(root))
         return casted->model();
      return nullptr;
   }
   extern QAbstractItemModel* get_underlying_model_of(QWidget* root) {
      auto* model = get_model_of(root);
      if (model) {
         while (auto* proxy = qobject_cast<QSortFilterProxyModel*>(model))
            model = proxy->sourceModel();
      }
      return model;
   }
}

