#include "DKHeaderView.h"
#include <QResizeEvent>

//
// QHeaderView isn't a very good widget. The controls for section sizes and 
// user resizing are unintuitive, poorly documented, and generally not very 
// usable at all.
// 
// When you  set a section's resize mode,  you are controlling  whether the 
// resize handle  on the  section's right edge  can be used  to resize that 
// section  and, if not, how the section resizes itself. This  introduces a 
// number of limitations. Consider, for example, a view with three columns, 
// wherein the middle column should stretch, but the user should be able to 
// enlarge the other two columns and  shrink the middle column. QHeaderView 
// will allow you to make the middle column stretch, but that then disables 
// its right-side resize handle (that  is, the rightmost column's left-side 
// resize handle).
// 
// DKHeaderView, by contrast, allows you to control columns' baseline sizes 
// as well as their sizes relative to  each other. Each column can be given 
// "flex" values comparable to those in CSS (i.e. grow and shrink factors), 
// and the header will do what it can to ensure that columns comply.
//

DKHeaderView::DKHeaderView(Qt::Orientation o, QWidget* parent) : QHeaderView(o, parent) {
   connect(this, &QHeaderView::sectionResized, this, [this](int index, int prior, int after) {
      if (this->_flexInProgress)
         return;
      if (index < 0)
         return;
      auto& list = this->_flexColumns;
      if (index >= list.size())
         return;
      if (after <= 0) {
         //
         // QHeaderView::setSectionHidden calls QHeaderView::resizeSection to force a section to 
         // zero-size, and does so before actually hiding it. As far as I can discern, it relies 
         // on the section being zero-size to actually prevent it from drawing.
         //
         list[index].hide = true;
         return;
      }
      list[index].hide = false;
      //
      int mod = after - list[index].render;

      int index_of_next = this->nextVisibleLogicalSection(index);
      if (index_of_next >= 0) {
         //
         // The user resized this section; that is, they clicked on the right edge of this 
         // section and dragged it... but the right edge of this section is the left edge 
         // of the next section. Would it be more sensible to resize that one instead?
         // 
         // If we choose to modify the next section, then we need to apply (-mod) to it.
         //
         auto& self = list[index];
         auto& next = list[index_of_next];
         if (self.grow > 0 && next.grow == 0) {
            //
            // If the current section stretches but the next section doesn't, then the user's 
            // attempt at resizing the two sections should be seen as modifying the latter, 
            // not the former.
            //
            next.mod -= mod;
         } else if (mod < 0 && next.mod < 0) {
            //
            // Making the current section smaller could be equivalent to making the next 
            // section bigger. If the next section has been resized below its basis, then 
            // let's do that instead.
            //
            next.mod -= mod;
         } else {
            self.mod += mod;
         }
      } else {
         auto& self = list[index];
         self.mod += mod;
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

void DKHeaderView::setFlexResizeEnabled(bool s) {
   this->_flexResizeEnabled = true;
}

int DKHeaderView::totalRenderedColumnSizes() const noexcept {
   int total = 0;
   for (int i = 0; i < this->_flexColumns.size(); ++i) {
      auto& entry = this->_flexColumns[i];
      if (entry.hide || this->isSectionHidden(i))
         continue;
      total += entry.render;
   }
   return total;
}
int DKHeaderView::visibleLength() const noexcept {
   auto* vp = this->viewport();
   if (!vp)
      return 0;
   int size;
   if (this->orientation() == Qt::Horizontal)
      return vp->width();
   return vp->height();
}

int DKHeaderView::columnBasisFactor(int logicalIndex) const noexcept {
   if (logicalIndex < 0)
      return 0;
   auto& list  = this->_flexColumns;
   auto  count = this->count();
   if (logicalIndex >= count)
      return 0;
   if (logicalIndex >= list.size())
      return 1;
   return list[logicalIndex].basis;
}
int DKHeaderView::columnModFactor(int logicalIndex) const noexcept {
   if (logicalIndex < 0)
      return 0;
   auto& list  = this->_flexColumns;
   auto  count = this->count();
   if (logicalIndex >= count)
      return 0;
   if (logicalIndex >= list.size())
      return 1;
   return list[logicalIndex].mod;
}
void DKHeaderView::setColumnModFactor(int logicalIndex, int factor) {
   if (logicalIndex < 0)
      return;
   auto& list  = this->_flexColumns;
   auto  count = this->count();
   if (logicalIndex >= count)
      return;
   if (list.size() < count)
      list.resize(count);
   list[logicalIndex].mod = factor;
}
int DKHeaderView::columnGrowFactor(int logicalIndex) const noexcept {
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
void DKHeaderView::setColumnGrowFactor(int logicalIndex, int factor) {
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
int DKHeaderView::columnShrinkFactor(int logicalIndex) const noexcept {
   if (logicalIndex < 0)
      return 0;
   auto& list  = this->_flexColumns;
   auto  count = this->count();
   if (logicalIndex >= count)
      return 0;
   if (logicalIndex >= list.size())
      return 0;
   return list[logicalIndex].shrink;
}
void DKHeaderView::setColumnShrinkFactor(int logicalIndex, int factor) {
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
void DKHeaderView::setColumnFlex(int logicalIndex, int grow, int shrink, int basis) {
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

void DKHeaderView::reapplyColumnFlex() {
   this->_reapplyColumnFlex(this->visibleLength());
}
void DKHeaderView::_reapplyColumnFlex(int length) {
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
      if (this->isSectionHidden(i) || entry.hide)
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
         auto& entry = this->_flexColumns[i];
         if (this->isSectionHidden(i) || entry.hide)
            continue;
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
      if (total_factor == 0) {
         //
         // We want to [grow|shrink], but we cannot.
         //
         for (int i = 0; i < count; ++i) {
            auto& entry = this->_flexColumns[i];
            if (this->isSectionHidden(i) || entry.hide)
               continue;
            entry.render = std::max(minimum_size, entry.basis + entry.mod);
            total_render += entry.render;
         }
      } else {
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
            auto& entry = this->_flexColumns[i];
            if (this->isSectionHidden(i) || entry.hide)
               continue;
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
   }
   //
   // Apply all computed sizes:
   //
   for (int i = 0; i < count; ++i) {
      auto& entry = this->_flexColumns[i];
      if (this->isSectionHidden(i) || entry.hide)
         continue;
      QHeaderView::resizeSection(i, entry.render);
   }
   //
   this->_flexInProgress = false;
}
void DKHeaderView::resizeSection(int logicalIndex, int size) {
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
void DKHeaderView::modSectionSizeTo(int logicalIndex, int size) {
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
   auto prior = this->sectionSize(logicalIndex);
   list[logicalIndex].mod += size - prior; // += because we should already be applying a `mod`
   this->reapplyColumnFlex();
}

int DKHeaderView::nextVisibleLogicalSection(int afterLogicalIndex) const {
   if (afterLogicalIndex < 0)
      return -1;

   auto count = this->count();
   if (afterLogicalIndex >= count)
      return -1;

   int subject_vis = this->visualIndex(afterLogicalIndex);
   if (subject_vis == -1)
      return -1;

   for (int i = subject_vis + 1; i < count; ++i) {
      auto logical = this->logicalIndex(i);
      if (logical == -1)
         continue;
      if (this->isSectionHidden(logical))
         continue;
      return logical;
   }
   return -1;
}

void DKHeaderView::resizeEvent(QResizeEvent* event) {
   if (!this->_flexResizeEnabled) {
      QHeaderView::resizeEvent(event);
      return;
   }
   this->reapplyColumnFlex();
}
void DKHeaderView::showEvent(QShowEvent* event) {
   QHeaderView::showEvent(event);
   if (!this->_flexResizeEnabled) {
      return;
   }
   this->reapplyColumnFlex();
}