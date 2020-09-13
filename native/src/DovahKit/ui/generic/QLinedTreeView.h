#pragma once
#include <QProxyStyle>
#include <QTreeView>

class QLinedTreeView : public QTreeView {
   Q_OBJECT
   protected:
      class _BranchStyle : public QProxyStyle {
         public:
            using QProxyStyle::QProxyStyle;
            void drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* p, const QWidget* w) const override;
      };
   public:
      QLinedTreeView(QWidget* parent) : QTreeView(parent) {
         this->setStyle(new _BranchStyle(this->style()));
      };
};
