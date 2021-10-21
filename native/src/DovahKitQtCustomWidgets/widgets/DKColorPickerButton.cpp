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
#include "DKColorPickerButton.h"
#include <QApplication>
#include <QColorDialog>
#include <QPaintEvent>
#include <QStyleOptionFocusRect>
#include <QStyleOptionButton>
#include <QStylePainter>

namespace {
   constexpr bool use_qss = false;
   constexpr bool use_default_palette = true;

   const QString _style("QPushButton { background-color: %1; color: %2; }");

   int _perceived_brightness(const QColor& color) {
      return sqrt(
         (color.red()   * color.red()   * 0.241F)
       + (color.green() * color.green() * 0.691F)
       + (color.blue()  * color.blue()  * 0.068F)
      );
   }
}
DKColorPickerButton::DKColorPickerButton(QWidget* parent) : QPushButton(parent) {
   #if !defined(QT_DESIGNER_LIB)
      QObject::connect(this, &QPushButton::clicked, [this]() {
         auto color = QColorDialog::getColor(this->_color, this->window(), QString(), (QColorDialog::ColorDialogOptions)(this->hasAlpha() ? QColorDialog::ShowAlphaChannel : 0));
         if (!color.isValid())
            return;
         this->setColor(color);
      });
   #endif
   this->ensurePolished();
   this->_updateColor();
}

QString DKColorPickerButton::colorName() const noexcept {
   return this->_color.name(this->hasAlpha() ? QColor::NameFormat::HexArgb : QColor::NameFormat::HexRgb).toUpper();
}

void DKColorPickerButton::setColor(QColor c) {
   this->_color = c;
   this->_updateColor();
   this->update();
   emit this->colorChanged(c);
}
void DKColorPickerButton::setHasAlpha(bool s) {
   if (this->_hasAlpha == s)
      return;
   this->_hasAlpha = s;
   this->setText(this->colorName());
}
void DKColorPickerButton::_updateColor() {
   constexpr int text_cutoff = 125;
   //
   if constexpr (use_qss) {
      auto text = QColor{ 0, 0, 0, 255 };
      if (_perceived_brightness(this->_color) < text_cutoff)
         text = QColor{ 255, 255, 255, 255 };
      this->setStyleSheet(
         _style
            .arg(this->_color.name(QColor::NameFormat::HexRgb))
            .arg(text.name(QColor::NameFormat::HexRgb))
      );
   } else {
      auto role = this->isEnabled() ? QPalette::ColorGroup::Normal : QPalette::ColorGroup::Disabled;
      bool swap = (_perceived_brightness(this->_color) < text_cutoff);
      //
      auto wp = this->palette(); // widget palette
      if constexpr (use_default_palette) {
         auto dp = QApplication::palette(this); // default palette
         if (swap) {
            wp.setColor(QPalette::ColorGroup::Normal,   QPalette::ColorRole::ButtonText, dp.color(QPalette::ColorGroup::Normal,   QPalette::ColorRole::BrightText));
            wp.setColor(QPalette::ColorGroup::Disabled, QPalette::ColorRole::ButtonText, dp.color(QPalette::ColorGroup::Disabled, QPalette::ColorRole::BrightText));
         } else {
            wp.setColor(QPalette::ColorGroup::Normal,   QPalette::ColorRole::ButtonText, dp.color(QPalette::ColorGroup::Normal,   QPalette::ColorRole::ButtonText));
            wp.setColor(QPalette::ColorGroup::Disabled, QPalette::ColorRole::ButtonText, dp.color(QPalette::ColorGroup::Disabled, QPalette::ColorRole::ButtonText));
         }
      } else {
         auto normal = QColor( 0,  0,  0);
         auto faded  = QColor(48, 48, 48);
         if (swap) {
            normal = QColor(255, 255, 255);
            faded  = QColor(192, 192, 192);
         }
         wp.setColor(QPalette::ColorGroup::Normal,   QPalette::ColorRole::ButtonText, normal);
         wp.setColor(QPalette::ColorGroup::Disabled, QPalette::ColorRole::ButtonText, faded);
      }
      this->setPalette(wp);
   }
   this->setText(this->colorName());
}

void DKColorPickerButton::paintEvent(QPaintEvent* event) {
   if constexpr (use_qss) {
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
      auto c = this->color();
      if (!this->_hasAlpha)
         c.setAlpha(255);
      //
      painter.save();
      painter.setPen(Qt::NoPen);
      painter.setBrush(c);
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