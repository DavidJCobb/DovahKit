#include "QHeaderViewDKEx.h"
#include <vector>
#include <QResizeEvent>

QHeaderViewDKEx::QHeaderViewDKEx(Qt::Orientation o, QWidget* parent) : QHeaderView(o, parent) {
   connect(this, &QHeaderView::sectionResized, this, [this](int index, int prior, int after) {
      if (this->_flexInProgress)
         return;
      if (index < 0)
         return;
      auto& list = this->_flexColumns;
      if (index >= list.size())
         return;
      list[index].mod += (after - list[index].render);
      if (list[index].mod < 0 || after < list[index].basis) {
         list[index].mod = 0;
      }
      int  length  = this->totalRenderedColumnSizes();
      int  visible = this->visibleLength();
      this->reapplyColumnFlex();
      if (length > visible) {
         auto resized = this->totalRenderedColumnSizes();
         if (resized < length) {
            //
            // The user had previously resized one or more columns such that it's no longer possible 
            // to fit the entire header in the viewport, and after having done so, they have reduced 
            // the size of a column. This *can* cause the scroll position to pull back, but it won't 
            // *always* do so, which means that there can be an empty margin left behind.
            //
            // Notably, in these cases, QHeaderView::length may remain "stuck" past the end of the 
            // header as well.
            //
            // This fix isn't perfect; there are still some margins and bad sizing that can be left 
            // behind once the user has resized the column widths back to within the viewport, but 
            // it's good enough and I've already spent hours on this. It's not worth any more time.
            //
            this->setOffsetToLastSection();
         }
      }
   });
}

void QHeaderViewDKEx::setFlexResizeEnabled(bool s) {
   this->_flexResizeEnabled = true;
}

int QHeaderViewDKEx::totalRenderedColumnSizes() const noexcept {
   int total = 0;
   for (auto& entry : this->_flexColumns)
      total += entry.render;
   return total;
}
int QHeaderViewDKEx::visibleLength() const noexcept {
   auto* vp = this->viewport();
   if (!vp)
      return 0;
   int size;
   if (this->orientation() == Qt::Horizontal)
      return vp->width();
   return vp->height();
}

int QHeaderViewDKEx::columnGrowFactor(int logicalIndex) const noexcept {
   if (logicalIndex < 0)
      return 0;
   auto& list  = this->_flexColumns;
   auto  count = this->count();
   if (logicalIndex >= count)
      return 0;
   if (logicalIndex >= list.size())
      return 1;
   return list[logicalIndex].grow;
}
void QHeaderViewDKEx::setColumnGrowFactor(int logicalIndex, int factor) {
   if (logicalIndex < 0)
      return;
   auto& list  = this->_flexColumns;
   auto  count = this->count();
   if (logicalIndex >= count)
      return;
   if (list.size() < count)
      list.resize(count);
   list[logicalIndex].grow = factor;
}
int QHeaderViewDKEx::columnShrinkFactor(int logicalIndex) const noexcept {
   if (logicalIndex < 0)
      return 0;
   auto& list = this->_flexColumns;
   auto  count = this->count();
   if (logicalIndex >= count)
      return 0;
   if (logicalIndex >= list.size())
      return 0;
   return list[logicalIndex].shrink;
}
void QHeaderViewDKEx::setColumnShrinkFactor(int logicalIndex, int factor) {
   if (logicalIndex < 0)
      return;
   auto& list  = this->_flexColumns;
   auto  count = this->count();
   if (logicalIndex >= count)
      return;
   if (list.size() < count)
      list.resize(count);
   list[logicalIndex].shrink = factor;
}
void QHeaderViewDKEx::setColumnFlex(int logicalIndex, int grow, int shrink, int basis) {
   if (logicalIndex < 0)
      return;
   auto& list  = this->_flexColumns;
   auto  count = this->count();
   if (logicalIndex >= count)
      return;
   if (list.size() < count)
      list.resize(count);
   auto& entry = list[logicalIndex];
   entry.grow   = grow;
   entry.shrink = shrink;
   if (basis >= 0)
      entry.basis = basis;
}

void QHeaderViewDKEx::reapplyColumnFlex() {
   this->_reapplyColumnFlex(this->visibleLength());
}
void QHeaderViewDKEx::_reapplyColumnFlex(int length) {
   int count = this->count();
   if (this->_flexColumns.size() < count)
      this->_flexColumns.resize(count);
   //
   int minimum_size = this->minimumSectionSize();
   int total_basis  = 0;
   int total_grow   = 0;
   int total_shrink = 0;
   int flex_count   = 0;
   for (int i = 0; i < count; ++i) {
      auto& entry = this->_flexColumns[i];
      entry.render = 0;
      if (this->isSectionHidden(i))
         continue;
      total_basis  += std::max(minimum_size, entry.basis + entry.mod);
      total_grow   += entry.grow;
      total_shrink += entry.shrink;
      if (entry.grow + entry.shrink)
         ++flex_count;
   }
   //
   this->_flexInProgress = true;
   int total_render = 0;
   //
   // Compute all column widths. We will apply them later, after corrections.
   //
   if (total_basis == length) {
      for (int i = 0; i < count; ++i) {
         if (this->isSectionHidden(i))
            continue;
         auto& entry = this->_flexColumns[i];
         entry.render = std::max(minimum_size, entry.basis + entry.mod);
         total_render += entry.render;
      }
   } else {
      //
      // We want to use the same basic approach, the same basic instructions, for the case 
      // where we need to grow columns and the case where we need to shrink columns. These 
      // two cases are largely mirrors of each other. So, we'll accomplish this with some 
      // temporary variables and a pointer-to-member.
      //
      // Note that we only apply *either of* flex grow or flex shrink, depending on whether 
      // there is extra space or insufficient space, respectively.
      //
      int _column_flex_info::* factor;
      int sign;
      int total_factor;
      if (length > total_basis) {
         factor = &_column_flex_info::grow;
         sign   = 1;
         total_factor = total_grow;
      } else {
         factor = &_column_flex_info::shrink;
         sign   = -1;
         total_factor = total_shrink;
      }
      //
      int    diff = length - total_basis;
      double per  = (double)diff / total_factor;
      //
      // We need to account for integer rounding errors on these calculations, which would 
      // result in there being leftover pixels at the end of the header viewport. If we just 
      // distribute these pixels into arbitrary sections, then we'll end up with jittering 
      // when the user resizes a section.
      //
      // What we need to do instead is actively carry the rounding error from one section 
      // into the next section. If rounding makes one section 0.33 pixels larger, then it 
      // should make the next section 0.33 pixels smaller, and vice versa, rather than us 
      // adding whole pixels to what are essentially random and irrelevant sections.
      //
      double carry = 0; // helper for sub-pixel values, to prevent jittering
      //
      for (int i = 0; i < count; ++i) {
         if (this->isSectionHidden(i))
            continue;
         auto&  entry  = this->_flexColumns[i];
         int    basis  = std::max(minimum_size, entry.basis + entry.mod);
         double offset = (per * (entry.*factor)) + carry;
         //
         double rounded = round(offset);
         entry.render = basis + rounded;
         if (entry.render < minimum_size) {
            entry.render = minimum_size;
            carry = offset - minimum_size;
         } else {
            carry = offset - rounded; // The effect of this is that if we round one column up by 0.33px, the next will have its computed width reduced by 0.33px.
         }
         //
         total_render += entry.render;
      }
   }
   //
   // Apply all computed sizes:
   //
   for (int i = 0; i < count; ++i) {
      if (this->isSectionHidden(i))
         continue;
      auto& entry = this->_flexColumns[i];
      QHeaderView::resizeSection(i, entry.render);
   }
   //
   this->_flexInProgress = false;
}
void QHeaderViewDKEx::resizeSection(int logicalIndex, int size) {
   {
      if (size < 0)
         return;
      auto min = this->minimumSectionSize();
      if (size > this->maximumSectionSize())
         return;
      if (size < min)
         size = min;
   }
   auto  count = this->count();
   auto& list  = this->_flexColumns;
   if (logicalIndex < 0 || logicalIndex >= count)
      return;
   if (list.size() < count)
      list.resize(count);
   list[logicalIndex].basis = size;
   list[logicalIndex].mod   = 0;
   this->reapplyColumnFlex();
}

void QHeaderViewDKEx::resizeEvent(QResizeEvent* event) {
   if (!this->_flexResizeEnabled) {
      QHeaderView::resizeEvent(event);
      return;
   }
   this->reapplyColumnFlex();
}