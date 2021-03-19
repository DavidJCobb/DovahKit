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
      this->reapplyColumnFlex();
   });
}

void QHeaderViewDKEx::setFlexResizeEnabled(bool s) {
   this->_flexResizeEnabled = true;
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
   auto* vp = this->viewport();
   if (!vp)
      return;
   int size;
   if (this->orientation() == Qt::Horizontal)
      size = vp->width();
   else
      size = vp->height();
   this->_reapplyColumnFlex(size);
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
   double carry = 0;
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
      double carry = 0; // helper for sub-pixel values, to prevent jittering
      for (int i = 0; i < count; ++i) {
         if (this->isSectionHidden(i))
            continue;
         auto&  entry  = this->_flexColumns[i];
         int    basis  = std::max(minimum_size, entry.basis + entry.mod);
         double offset = sign * (per * (entry.*factor)) + carry;
         //
         double rounded = round(offset);
         carry = offset - rounded; // The effect of this is that if we round one column up by 0.33px, the next will have its computed width reduced by 0.33px.
         //
         entry.render = basis + rounded;
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