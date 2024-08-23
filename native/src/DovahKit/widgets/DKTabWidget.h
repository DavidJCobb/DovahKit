#pragma once
#include <QTabWidget>

// Exists to make use of the bugfixes in DKTabBar.
class DKTabWidget : public QTabWidget {
   Q_OBJECT;
   public:
      DKTabWidget(QWidget* parent = nullptr);

      void setTabBar(QTabBar*);

      void setTabVisible(int index, bool visible) {
         //
         // QTabBar in Qt 5 is riddled with bugs: there are tons of places in the codebase 
         // where the authors just forget to check whether certain tabs are visible, and 
         // some of these bugs cause massive visual disruptions. For example, if you hide 
         // the last tab, then QTabBar becomes completely unable to calculate the correct 
         // scroll position needed to force a tab into view (QTabBarPrivate::makeVisible). 
         // It accidentally bases its calculations on the null rect used by the hidden tab, 
         // and ends up computing a massively negative scroll position that right-aligns 
         // the currently selected tab.
         // 
         // Fixing these bugs is impossible due to Qt's use of PImpl. The only option is to 
         // build complete replacements for both QTabWidget and QTabBar from scratch, which 
         // I don't have time to do right now. Instead, we'll use enable state instead of 
         // visibility state (there are bugs there too, but so far I've been able to fix 
         // those with the DKTabBar subclass).
         //
         this->setTabEnabled(index, visible);
      }
};
