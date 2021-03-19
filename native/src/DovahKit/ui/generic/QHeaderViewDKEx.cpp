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
   if (total_basis == length) {
      for (int i = 0; i < count; ++i) {
         if (this->isSectionHidden(i))
            continue;
         auto& entry = this->_flexColumns[i];
         entry.render = std::max(minimum_size, entry.basis + entry.mod);
         total_render += entry.render;
      }
   } else if (total_basis < length) {
      int    extra   = length - total_basis;
      double consume = extra / total_grow;
      for (int i = 0; i < count; ++i) {
         if (this->isSectionHidden(i))
            continue;
         auto& entry = this->_flexColumns[i];
         int   basis = std::max(minimum_size, entry.basis + entry.mod);
         int   grow  = entry.grow * consume;
         entry.render = basis + grow;
         total_render += entry.render;
      }
   } else {
      int    excess = total_basis - length;
      double recede = (double)excess / total_shrink;
      for (int i = 0; i < count; ++i) {
         if (this->isSectionHidden(i))
            continue;
         auto& entry = this->_flexColumns[i];
         int   basis  = std::max(minimum_size, entry.basis + entry.mod);
         int   shrink = entry.shrink * recede;
         entry.render = basis - shrink;
         total_render += entry.render;
      }
   }
   //
   // Account for leftover pixels due to integer rounding:
   //
   if (total_render < length) {
      int extra = length - total_render;
      if (extra < flex_count) {
         int diff = extra / flex_count;
         if (diff > 0) {
            //
            // Distribute the extra pixels into the flexible columns.
            //
            for (int i = 0; i < count; ++i) {
               if (this->isSectionHidden(i))
                  continue;
               auto& entry = this->_flexColumns[i];
               if (entry.grow + entry.shrink)
                  entry.render += diff;
            }
         }
         //
         // Distribute any final remaining pixels into the first column.
         //
         extra -= (diff * flex_count);
         if (extra > 0) {
            this->_flexColumns[0].render += extra;
         }
      } else if (count) {
         int diff = extra / count;
         if (diff > 0) {
            //
            // Distribute the extra pixels into the first few columns.
            //
            for (int i = 0; i < count; ++i) {
               if (this->isSectionHidden(i))
                  continue;
               auto& entry = this->_flexColumns[i];
               entry.render += diff;
            }
         }
         //
         // Distribute any final remaining pixels into the first column.
         //
         extra -= (diff * count);
         if (extra > 0) {
            this->_flexColumns[0].render += extra;
         }
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