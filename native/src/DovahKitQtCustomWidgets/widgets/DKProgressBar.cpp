#include "./DKProgressBar.h"
#include <cstdint>
#include <QLocale>
#include <QStylePainter>
#include <QStyleOptionProgressBar>

bool DKProgressBar::isIndeterminate() const {
   if (this->minimum() != 0)
      return false;
   if (this->maximum() != 0)
      return false;
   return true;
}
bool DKProgressBar::isReset() const {
   const auto min = this->minimum();
   const auto val = this->value();
   //
   // If the minimum is INT_MIN, then the sentinel value for "reset" is INT_MIN. 
   // Otherwise, it is the minimum minus one.
   //
   if (val < min)
      return true;
   if (val == INT_MIN && min == INT_MIN)
      return true;
   return false;
}

void DKProgressBar::makeIndeterminate() {
   this->setRange(0, 0);
}

void DKProgressBar::setMinimum(int v) {
   this->setRange(v, std::max(v, this->maximum()));
}
void DKProgressBar::setMaximum(int v) {
   this->setRange(std::min(v, this->minimum()), v);
}
void DKProgressBar::setRange(int min, int max) {
   auto prior_value = this->value();
   bool was_reset   = this->isReset();

   // If the prior value is outside of this range, then the progress bar will be 
   // reset... but at least in Qt 5, QProgressBar doesn't emit `valueChanged` when 
   // changing the value as part of a `reset` operation. So we don't need to do 
   // complicated maneuvering to prevent that signal from going out.
   QProgressBar::setRange(min, max);
   if (!was_reset) {
      if (prior_value < min)
         this->setValue(min);
      else if (prior_value > max)
         this->setValue(max);
   }
}

namespace {
   // in Qt 5, QLatin1String::QLatin1String(const char*) is incorrectly marked constexpr despite 
   // invoking strlen, which is not constexpr.
   static constexpr auto _construct_latin_1_string(std::string_view v) {
      return QLatin1String(v.data(), v.size());
   }
}

/*
   The default QProgressBar::text() implementation refuses to display any text if 
   the progress bar is in a "reset" or "indeterminate" state. This happens even if 
   your chosen format string doesn't try to display any values.

   Qt intended for that to be convenient, since the progress bar can enter a "reset" 
   state as a side-effect of configuration changes e.g. its range. However, applying 
   this behavior to the "indeterminate" state is, frankly, stupid, since you'll only 
   end up in that state as a result of a specific action, and if you want the bar to 
   be cleared at that time, you can set the format string and clear it yourself.

   So, we have to reimplement basically the entire "text" logic to *not* do that.
*/
/*virtual*/ QString DKProgressBar::text() const /*override*/ {
   constexpr const auto token_max = _construct_latin_1_string("%m");
   constexpr const auto token_val = _construct_latin_1_string("%v");
   constexpr const auto token_pct = _construct_latin_1_string("%p");

   QString result = this->format();
   if (this->isIndeterminate()) {
      result.replace(token_max, tr("?"));
      result.replace(token_val, tr("?"));
      result.replace(token_pct, tr("?"));
      return result;
   }
   const auto min = this->minimum();
   const auto max = this->maximum();
   auto val = this->value();
   if (this->isReset()) {
      val = 0;
      if (min == INT_MIN) {
         result.replace(token_max, tr("?"));
         result.replace(token_val, tr("?"));
         result.replace(token_pct, tr("?"));
         return result;
      }
   }

   // QProgressBar stringifies numbers without using locale options, for compatibility.
   QLocale locale = this->locale();
   locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);

   auto step_count = (int64_t)max - min;
   result.replace(token_max, locale.toString(step_count));
   result.replace(token_val, locale.toString(val));
   {
      int percentage;
      if (step_count == 0) {
         percentage = 100;
      } else {
         percentage = ((int64_t)val - min) * 100.0 / step_count;
      }
      result.replace(token_pct, locale.toString(percentage));
   }
   return result;
}

/*virtual*/ void DKProgressBar::paintEvent(QPaintEvent* event) /*override*/ {
   if (this->isTextVisible() && (this->isIndeterminate() || this->isReset())) {
      //
      // HACK: when the progress bar text is centered, Qt's style classes check whether 
      // the text needs to be displayed in an alternate color based on whether the bar 
      // is more than half full. When the bar is reset or indeterminate, that check ends 
      // up passing, when we don't want it to.
      // 
      // Easiest way to remedy that is to forcibly disable text drawing, and then draw 
      // the text ourselves, passing in B.S. params.
      //
      QStylePainter paint(this);
      QStyleOptionProgressBar opt;
      this->initStyleOption(&opt);
      opt.textVisible = false;
      paint.drawControl(QStyle::CE_ProgressBar, opt);
      {
         auto subopt = opt;
         subopt.rect = this->style()->subElementRect(QStyle::SE_ProgressBarLabel, &opt, this);
         subopt.minimum  = 0;
         subopt.maximum  = 100;
         subopt.progress = 0;
         paint.drawControl(QStyle::CE_ProgressBarLabel, subopt);
      }
      return;
   }
   QProgressBar::paintEvent(event);
}