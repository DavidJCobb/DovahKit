#include "./DKComboBox.h"
#include <utility>
#include <QStyleOptionComboBox>

void DKComboBox::setAutoResizeEnabled(bool v) {
   if (v == this->autoResizeEnabled())
      return;
   this->_state.auto_resize_enabled = v;
   {
      auto len = this->_state.desired_min_contents_length;
      if (!v && len == 0)
         len = 1;
      this->setMinimumContentsLength(len);
   }
   this->updateGeometry();
}
void DKComboBox::setMinimumContentsLength(int v) {
   if (v == this->_state.desired_min_contents_length)
      return;
   this->_state.desired_min_contents_length = v;

   if (!this->_state.auto_resize_enabled && v == 0)
      v = 1;
   this->setMinimumContentsLength(v);
}

/*virtual*/ QSize DKComboBox::sizeHint() const /*override*/ {
   if (this->_state.auto_resize_enabled)
      return QComboBox::sizeHint();
   //
   // Per Qt's source code, `sizeHint` and `minimumSizeHint` use the same underlying function, 
   // but only the former takes the combobox's contents (besides the presence of an icon on 
   // any list item) into account. By routing `sizeHint` to `minimumSizeHint`, we can force 
   // the combobox to stop expanding to fit its content.
   // 
   // Note that if `minimumContentsLength` is zero, then `minimumSizeHint` behaves the same as
   // `sizeHint` i.e. it forces the widget to expand to fit its longest item. The function will 
   // guard against negative minimums, but only disables the expansion behavior if the minimum 
   // is zero. This behavior isn't documented anywhere and isn't part of the contract, because 
   // of course it isn't
   //
   //return this->minimumSizeHint(); // LOL LMAO EVEN THAT DOESN'T BLOODY WORK

   QSize size_hint;
   {
      auto height   = std::max(14, this->fontMetrics().height()) + 2;
      bool has_icon = false;
      if (this->sizeAdjustPolicy() == QComboBox::SizeAdjustPolicy::AdjustToMinimumContentsLengthWithIcon) {
         has_icon = true;
      } else {
         for (size_t i = 0; i < this->count(); ++i) {
            if (!this->itemIcon(i).isNull()) {
               has_icon = true;
               break;
            }
         }
      }
      if (has_icon) {
         auto icon_height = this->iconSize().height();
         if (icon_height > height)
            height = icon_height;
      }
      size_hint.setHeight(height);
   }

   QStyleOptionComboBox opt;
   this->initStyleOption(&opt);
   size_hint = this->style()->sizeFromContents(QStyle::ContentsType::CT_ComboBox, &opt, size_hint, this);
   return size_hint;
}
/*virtual*/ QSize DKComboBox::minimumSizeHint() const /*override*/ {
   if (this->_state.auto_resize_enabled)
      return QComboBox::minimumSizeHint();
   return this->sizeHint();
}