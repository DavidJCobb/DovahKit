#pragma once
#include <QWidget>
#include "BoundInput.h"

class DK3DBindingOptionsContainerWidget : public QWidget {
   Q_OBJECT;
   public:
      using QWidget::QWidget;

   public slots:
      void clear();

   signals:
      void boundInputChanged(const DK3D::BoundInput&);
};