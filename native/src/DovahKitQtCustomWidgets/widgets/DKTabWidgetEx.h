#pragma once
#include <QIcon>
#include <QStackedWidget>
#include <QString>
#include <QTabWidget> // for enums
#include <QWidget>
#include "./DKTabBarEx.h"

class DKTabBarEx;

class DKTabWidgetEx : public QWidget {
   Q_OBJECT;
   public:
      DKTabWidgetEx(QWidget* parent = nullptr);
      ~DKTabWidgetEx();

      using TabPosition = QTabWidget::TabPosition;
      using TabShape    = QTabWidget::TabShape;
      using TabInfo     = DKTabBarEx::TabInfo;

   public:
      #pragma region Properties
         constexpr TabPosition tabPosition() const noexcept { return this->_properties.tab_position; }
         void setTabPosition(TabPosition);

         constexpr TabShape tabShape() const noexcept { return this->_properties.tab_shape; }
         void setTabShape(TabShape);
      #pragma endregion

      #pragma region Tabbar properties (forwards to tabbar)
         bool tabBarAutoHide() const; // autoHide
         void setTabBarAutoHide(bool); // setAutoHide

         bool documentMode() const;
         void setDocumentMode(bool);

         Qt::TextElideMode elideMode() const;
         void setElideMode(Qt::TextElideMode);

         QSize iconSize() const;
         void setIconSize(const QSize&);

         bool isMovable() const;
         void setMovable(bool);

         bool tabsClosable() const;
         void setTabsClosable(bool);

         bool usesScrollButtons() const;
         void setUsesScrollButtons(bool);
      #pragma endregion

      #pragma region Per-tab properties (forwards to tabbar)
         bool isTabEnabled(size_t index) const;
         void setTabEnabled(size_t index, bool);

         bool isTabVisible(size_t index) const;
         void setTabVisible(size_t index, bool);

         QIcon tabIcon(size_t index) const;
         void setTabIcon(size_t index, QIcon);

         QString tabText(size_t index) const;
         void setTabText(size_t index, QString);

         QString tabToolTip(size_t index) const;
         void setTabToolTip(size_t index, QString);

         QString tabWhatsThis(size_t index) const;
         void setTabWhatsThis(size_t index, QString);
      #pragma endregion
      
      inline size_t addTab(QWidget* page, QString label) { return this->addTab(page, {}, label); }
      inline size_t addTab(QWidget* page, QIcon icon, QString label) { return this->insertTab(this->count(), page, icon, label); }
      void clear();
      inline size_t count() const { return this->_stack->count(); }
      inline size_t currentIndex() const { return (size_t)this->_stack->currentIndex(); }
      inline QWidget* currentWidget() const { return this->_stack->currentWidget(); }
      inline size_t indexOf(QWidget* widget) const { return this->_stack->indexOf(widget); }
      inline size_t insertTab(size_t index, QWidget* page, QString label) { return this->insertTab(index, page, {}, label); }
      size_t insertTab(size_t index, QWidget* page, QIcon, QString label);
      void removeTab(size_t);
      inline QWidget* widget(size_t index) const { return this->_stack->widget(index); }

      // Adds a new tab with a default name and no configuration options.
      // Necessary due to Qt Designer's limitations.
      void addTabBody(QWidget*);

      // Sets the info for all tabs. Necessary due to Qt Designer's limitations.
      void setAllTabInfos(const QList<TabInfo>&);

   protected:
      struct {
         TabPosition tab_position = TabPosition::North;
         TabShape    tab_shape    = TabShape::Rounded;
      } _properties;
      QStackedWidget* _stack  = nullptr;
      DKTabBarEx*     _tabbar = nullptr;

      // sync with tabbar
      void _on_current_tab_changed(size_t current);
      void _on_tab_close_requested(size_t);
      void _on_tab_moved(size_t from, size_t to);

      void _update_layout();
};
