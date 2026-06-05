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
#include "basic_bindings.h"

namespace cobb::qt {
   void bind(QCheckBox* widget, bool& target) {
      widget->setChecked(target);
      QObject::connect(widget, &QCheckBox::checkStateChanged, widget, [&target](int state) {
         target = state == Qt::CheckState::Checked;
      });
   }
   void unbind(QCheckBox* widget) {
      QObject::disconnect(widget, &QCheckBox::checkStateChanged, widget, nullptr);
   }

   void bind(QLineEdit* widget, std::string& target) {
      widget->setText(QString::fromUtf8(target.c_str()));
      QObject::connect(widget, &QLineEdit::textChanged, widget, [&target](const QString& value) {
         target = value.toUtf8().data();
      });
   }
   void unbind(QLineEdit* widget) {
      QObject::disconnect(widget, &QLineEdit::textChanged, widget, nullptr);
   }
}