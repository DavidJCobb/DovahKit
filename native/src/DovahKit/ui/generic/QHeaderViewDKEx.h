#pragma once
#include <QHeaderView>

class QHeaderViewDKEx : public QHeaderView {
   public:
      using QHeaderView::QHeaderView;
      
      inline bool flexResizeEnabled() const noexcept { return this->_flexResizeEnabled; }
      void setFlexResizeEnabled(bool);
      
      int  columnGrowFactor(int logicalIndex) const noexcept;
      void setColumnGrowFactor(int logicalIndex, int factor);
      int  columnShrinkFactor(int logicalIndex) const noexcept;
      void setColumnShrinkFactor(int logicalIndex, int factor);
      void setColumnFlex(int logicalIndex, int grow, int shrink = 0, int basis = -1);

      void reapplyColumnFlex();
      void resizeSection(int logicalIndex, int size);
      
   protected:
      void _reapplyColumnFlex(int length);
      void resizeEvent(QResizeEvent* event) override;

      struct _column_flex_info {
         int grow   = 1;
         int shrink = 0;
         int basis  = 0;
      };
      QVector<_column_flex_info> _flexColumns;

      struct {
         QVector<int> factors;
      } _flex;

      bool _flexResizeEnabled = false;
};
