#pragma once
#include <QTabWidget>

// Exists to make use of the bugfixes in DKTabBar.
class DKTabWidget : public QTabWidget {
   Q_OBJECT;
   public:
      DKTabWidget(QWidget* parent = nullptr);
};
