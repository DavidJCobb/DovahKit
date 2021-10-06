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
#include "ColorPickerButton.h"
#include <QColorDialog>
#include <QPaintEvent>
#include <QStyleOptionFocusRect>
#include <QStyleOptionButton>
#include <QStylePainter>

namespace {
   constexpr bool use_qss = false;

   const QString _style("QPushButton { background-color: %1; color: %2; }");

   int _perceived_brightness(const QColor& color) {
      return sqrt(
         (color.red()   * color.red()   * 0.241F)
       + (color.green() * color.green() * 0.691F)
       + (color.blue()  * color.blue()  * 0.068F)
      );
   }
}
ColorPickerButton::ColorPickerButton(QWidget* parent) : QPushButton(parent) {
   QObject::connect(this, &QPushButton::clicked, [this]() {
      auto color = QColorDialog::getColor(this->_color, this->window(), QString(), (QColorDialog::ColorDialogOptions)(this->hasAlpha() ? QColorDialog::ShowAlphaChannel : 0));
      if (!color.isValid())
         return;
      this->setColor(color);
   });
}
void ColorPickerButton::setColor(QColor c) {
   this->_color = c;
   this->_updateColor();
   emit this->colorChanged(c);
}
void ColorPickerButton::setHasAlpha(bool s) {
   this->_hasAlpha = s;
}
void ColorPickerButton::_updateColor() {
   constexpr int text_cutoff = 125;
   //
   QString code = this->_color.name();
   QColor  text = QColor{ 0, 0, 0, 255 };
   if (_perceived_brightness(this->_color) < text_cutoff)
      text = QColor{ 255, 255, 255, 255 };
   if (use_qss) {
      this->setStyleSheet(_style.arg(code).arg(text.name()));
   } else {
      auto p = this->palette();
      p.setColor(QPalette::ColorRole::ButtonText, text);
      this->setPalette(p);
   }
   this->setText(code);
}

void ColorPickerButton::paintEvent(QPaintEvent* event) {
   if (use_qss) {
      QPushButton::paintEvent(event);
      return;
   }
   QStyle* style = this->style();
   //
   QStylePainter      painter(this);
   QStyleOptionButton option;
   this->initStyleOption(&option);
   //
   painter.drawControl(QStyle::CE_PushButtonBevel, option);
   //
   QRect content_rect = style->subElementRect(QStyle::SE_PushButtonContents, &option, this);
   {
      painter.save();
      painter.setPen(Qt::NoPen);
      painter.setBrush(this->color());
      painter.drawRect(content_rect);
      painter.restore();
   }
   {
      auto sub = option;
      sub.rect = content_rect;
      painter.drawControl(QStyle::CE_PushButtonLabel, sub);
   }
   if (option.state & QStyle::State_HasFocus) {
      QStyleOptionFocusRect sub;
      sub.QStyleOption::operator=(option);
      sub.rect = style->subElementRect(QStyle::SE_PushButtonFocusRect, &option, this);
      painter.drawPrimitive(QStyle::PE_FrameFocusRect, sub);
   }
}