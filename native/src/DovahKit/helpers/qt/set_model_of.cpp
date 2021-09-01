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
#include "set_model_of.h"
#include <QAbstractItemView>
#include <QComboBox>

namespace cobb::qt {
   extern void set_model_of(QWidget* widget, QAbstractItemModel* model) {
      if (auto* casted = qobject_cast<QAbstractItemView*>(widget)) {
         casted->setModel(model);
         return;
      }
      if (auto* casted = qobject_cast<QComboBox*>(widget)) {
         casted->setModel(model);
         return;
      }
      #if _DEBUG
         __debugbreak(); // This widget can't have a model!
      #endif
   }
}

