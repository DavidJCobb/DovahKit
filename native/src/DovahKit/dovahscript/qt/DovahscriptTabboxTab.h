#pragma once
#include <QTabWidget>
#include <QWidget>

class DovahscriptTabboxTab : public QWidget {
   Q_OBJECT;
   public:
      using QWidget::QWidget;

      QTabWidget* containingTabWidget() const noexcept;
};