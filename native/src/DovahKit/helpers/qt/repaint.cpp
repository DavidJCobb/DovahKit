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
#include "repaint.h"
#include <QAbstractItemView>
#include <QComboBox>
#include <QScrollArea>

namespace cobb::qt {
   extern void update_hierarchy(QWidget* root) {
      root->update();
      auto desc = root->findChildren<QWidget*>();
      for (auto* d : desc) {
         d->update();
         if (auto* combobox = qobject_cast<QComboBox*>(d)) {
            if (auto* view = combobox->view())
               if (auto* viewport = view->viewport())
                  viewport->update();
         } else if (auto* scrollbox = qobject_cast<QScrollArea*>(d)) {
            if (auto* body = scrollbox->widget())
               update_hierarchy(body);
         }
      }
   }
}
