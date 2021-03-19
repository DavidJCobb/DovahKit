#include "QHeaderViewDKEx.h"
#include <vector>
#include <QResizeEvent>

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
   for (int i = 0; i < count; ++i) {
      if (this->isSectionHidden(i))
         continue;
      auto& entry = this->_flexColumns[i];
      total_basis  += std::max(minimum_size, entry.basis);
      total_grow   += entry.grow;
      total_shrink += entry.shrink;
   }
   //
   if (total_basis == length) {
      for (int i = 0; i < count; ++i) {
         if (this->isSectionHidden(i))
            continue;
         QHeaderView::resizeSection(i, std::max(minimum_size, this->_flexColumns[i].basis));
      }
      return;
   }
   if (total_basis < length) {
      int    extra   = length - total_basis;
      double consume = extra / total_grow;
      for (int i = 0; i < count; ++i) {
         if (this->isSectionHidden(i))
            continue;
         int basis = std::max(minimum_size, this->_flexColumns[i].basis);
         int grow  = this->_flexColumns[i].grow * consume;
         QHeaderView::resizeSection(i, basis + grow);
      }
      return;
   }
   int    excess = total_basis - length;
   double recede = (double)excess / total_shrink;
   for (int i = 0; i < count; ++i) {
      if (this->isSectionHidden(i))
         continue;
      int basis  = std::max(minimum_size, this->_flexColumns[i].basis);
      int shrink = this->_flexColumns[i].shrink * recede;
      QHeaderView::resizeSection(i, basis - shrink);
   }
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
   this->reapplyColumnFlex();
}

void QHeaderViewDKEx::resizeEvent(QResizeEvent* event) {
   if (!this->_flexResizeEnabled) {
      QHeaderView::resizeEvent(event);
      return;
   }
   this->reapplyColumnFlex();
}