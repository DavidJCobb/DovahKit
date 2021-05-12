#pragma once
#include <QHeaderView>

class QHeaderViewDKEx : public QHeaderView {
   public:
      QHeaderViewDKEx(Qt::Orientation, QWidget* parent = nullptr);
      
      inline bool flexResizeEnabled() const noexcept { return this->_flexResizeEnabled; }
      void setFlexResizeEnabled(bool);

      int totalRenderedColumnSizes() const noexcept;
      int visibleLength() const noexcept;
      
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
         int mod    = 0; // change made by the user when resizing columns; added to basis
         int render = 0; // last rendered size
      };
      QVector<_column_flex_info> _flexColumns;

      struct {
         QVector<int> factors;
      } _flex;
      bool _flexInProgress    = false;
      bool _flexResizeEnabled = false;
};
