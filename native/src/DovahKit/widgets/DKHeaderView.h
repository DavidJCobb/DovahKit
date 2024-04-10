#pragma once
#include <QHeaderView>

//
// A subclass of QHeaderView meant to allow programmers to control columns' sizes 
// relative to each other. Refer to the CPP file for details.
//
class DKHeaderView : public QHeaderView {
   public:
      DKHeaderView(Qt::Orientation, QWidget* parent = nullptr);
      
      inline bool flexResizeEnabled() const noexcept { return this->_flexResizeEnabled; }
      void setFlexResizeEnabled(bool);

      int totalRenderedColumnSizes() const noexcept;
      int visibleLength() const noexcept;
      
      int  columnBasisFactor(int logicalIndex) const noexcept;
      int  columnModFactor(int logicalIndex) const noexcept;
      void setColumnModFactor(int logicalIndex, int factor);
      int  columnGrowFactor(int logicalIndex) const noexcept;
      void setColumnGrowFactor(int logicalIndex, int factor);
      int  columnShrinkFactor(int logicalIndex) const noexcept;
      void setColumnShrinkFactor(int logicalIndex, int factor);
      void setColumnFlex(int logicalIndex, int grow, int shrink = 0, int basis = -1);

      void reapplyColumnFlex();
      void resizeSection(int logicalIndex, int size); // changes the basis
      void modSectionSizeTo(int logicalIndex, int size); // applies a modifier, as if the user resized the section

      int nextVisibleLogicalSection(int afterLogicalIndex) const;
      
   protected:
      void _reapplyColumnFlex(int length);
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;

      struct _column_flex_info {
         int  grow   = 1;
         int  shrink = 0;
         int  basis  = 0;
         int  mod    = 0; // change made by the user when resizing columns; added to basis
         int  render = 0; // last rendered size
         bool hide   = false; // compensate for badly-designed QHeaderView::setSectionHidden behavior
      };
      QVector<_column_flex_info> _flexColumns;

      struct {
         QVector<int> factors;
      } _flex;
      bool _flexInProgress    = false;
      bool _flexResizeEnabled = false;
};
