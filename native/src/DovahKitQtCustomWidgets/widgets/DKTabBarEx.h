#pragma once
#include <optional>
#include <vector>
#include <QColor>
#include <QIcon>
#include <QString>
#include <QTabBar>
#include <QVariant>
#include <QWidget>

class DKTabBarEx : public QWidget {
   Q_OBJECT;
   public:
      using ButtonPosition    = QTabBar::ButtonPosition;
      using SelectionBehavior = QTabBar::SelectionBehavior;
      using Shape             = QTabBar::Shape;

   public:
      struct TabInfo {
         QString accessibleName;
         bool    enabled = true;
         QIcon   icon;
         QString name;
         QString toolTip;
         bool    visible = true;
         QString whatsThis;
      };

   public:
      DKTabBarEx(QWidget* parent = nullptr);
      ~DKTabBarEx();

   public:
      #pragma region Properties
         constexpr bool autoHide() const noexcept { return this->_properties.auto_hide_if_solo_tab; }
         void setAutoHide(bool);

         constexpr bool changeCurrentOnDrag() const noexcept { return this->_properties.change_current_on_drag; }
         void setChangeCurrentOnDrag(bool);

         constexpr bool documentMode() const noexcept { return this->_properties.document_mode; }
         void setDocumentMode(bool);

         constexpr bool drawBase() const noexcept { return this->_properties.draw_base; }
         void setDrawBase(bool);

         constexpr Qt::TextElideMode elideMode() const noexcept { return this->_properties.elide_mode; }
         void setElideMode(Qt::TextElideMode);

         constexpr bool expanding() const noexcept { return this->_properties.expand_tab_widths; }
         void setExpanding(bool);

         constexpr QSize iconSize() const noexcept { return this->_properties.tab_icon_size; }
         void setIconSize(const QSize&);

         constexpr bool isMovable() const noexcept { return this->_properties.can_move_tabs; }
         constexpr void setMovable(bool v) { this->_properties.can_move_tabs = v; }

         constexpr SelectionBehavior selectionBehaviorOnRemove() const noexcept { return this->_properties.selection_behavior_on_remove; }
         constexpr void setSelectionBehaviorOnRemove(SelectionBehavior v) { this->_properties.selection_behavior_on_remove = v; }

         constexpr Shape shape() const noexcept { return this->_properties.tab_shape; }
         void setShape(Shape);

         constexpr bool tabsClosable() const noexcept { return this->_properties.can_close_tabs; }
         void setTabsClosable(bool);

         constexpr bool usesScrollButtons() const noexcept { return this->_properties.uses_scroll_buttons; }
         void setUsesScrollButtons(bool);
      #pragma endregion

      #pragma region Tab properties
         QString accessibleTabName(size_t) const;
         void setAccessibleTabName(size_t, QString);

         constexpr QWidget* tabButton(size_t, ButtonPosition) const;
         void setTabButton(size_t, ButtonPosition, QWidget*);

         QVariant tabData(size_t) const;
         void setTabData(size_t, const QVariant&);

         constexpr bool isTabEnabled(size_t) const;
         void setTabEnabled(size_t, bool);

         QIcon tabIcon(size_t) const;
         void setTabIcon(size_t, QIcon);

         QString tabText(size_t) const;
         void setTabText(size_t, QString);

         QColor tabTextColor(size_t) const;
         void setTabTextColor(size_t, QColor);

         QString tabToolTip(size_t) const;
         void setTabToolTip(size_t, QString);

         constexpr bool isTabVisible(size_t) const;
         void setTabVisible(size_t, bool);

         QString tabWhatsThis(size_t) const;
         void setTabWhatsThis(size_t, QString);
      #pragma endregion

      constexpr std::optional<size_t> currentIndex() const noexcept { return this->_state.current_index; }
      void setCurrentIndex(std::optional<size_t>);

      constexpr size_t count() const noexcept { return this->_tabs.size(); }

      std::optional<size_t> tabAt(const QPoint&) const;
      QRect tabRect(size_t) const;

      inline size_t addTab(QString text) { return this->addTab({}, text); };
      inline size_t addTab(QIcon icon, QString text) { return this->insertTab(this->count(), icon, text); }
      inline size_t insertTab(size_t index, QString text) { return this->insertTab(index, {}, text); }
      size_t insertTab(size_t, QIcon, QString);
      void moveTab(size_t from, size_t to);
      void removeTab(size_t);

   #pragma region Overrides
   public:
      virtual QSize minimumSizeHint() const override;
      virtual QSize sizeHint() const override;
   protected:
      virtual void changeEvent(QEvent* event) override;
      virtual bool event(QEvent* event) override;
      virtual void hideEvent(QHideEvent*) override;
      virtual void keyPressEvent(QKeyEvent* event) override;
      virtual void mouseMoveEvent(QMouseEvent* event) override;
      virtual void mousePressEvent(QMouseEvent* event) override;
      virtual void mouseReleaseEvent(QMouseEvent* event) override;
      virtual void paintEvent(QPaintEvent*) override;
      virtual void resizeEvent(QResizeEvent*) override;
      virtual void showEvent(QShowEvent*) override;
      virtual void timerEvent(QTimerEvent* event) override;
      virtual void wheelEvent(QWheelEvent* event) override;
   #pragma endregion

   signals:
      void currentChanged(std::optional<size_t>);
      void tabBarClicked(std::optional<size_t> tab_index);
      void tabBarDoubleClicked(std::optional<size_t> tab_index);
      void tabCloseRequested(size_t tab_index);
      void tabMoved(size_t from, size_t to);
      
   protected:
      struct _InternalTabInfo {
         struct {
            QWidget* left  = nullptr;
            QWidget* right = nullptr;
         } buttons;
         QVariant data;
         TabInfo  info;
         QRect    rect;
         QColor   text_color;
      };

      struct {
         bool  auto_hide_if_solo_tab  = false;
         bool  can_close_tabs         = false;
         bool  can_move_tabs          = false;
         bool  change_current_on_drag = false;
         bool  document_mode          = false;
         bool  draw_base              = false;
         bool  expand_tab_widths      = false;
         QSize tab_icon_size;
         Shape tab_shape              = Shape::RoundedNorth;
         bool  uses_scroll_buttons    = false;

         Qt::TextElideMode elide_mode = Qt::TextElideMode::ElideNone; // for tab names
         SelectionBehavior selection_behavior_on_remove = SelectionBehavior::SelectRightTab;
      } _properties;
      struct {
         std::optional<size_t> current_index = 0;
         int scroll_position = 0;
      } _state;
      std::vector<_InternalTabInfo*> _tabs;

      constexpr const _InternalTabInfo* _tab_info(size_t index) const noexcept {
         if (index < this->_tabs.size())
            return this->_tabs[index];
         return nullptr;
      }
      constexpr _InternalTabInfo* _tab_info(size_t index) noexcept {
         return const_cast<_InternalTabInfo*>(std::as_const(*this)._tab_info(index));
      }

      void _force_tab_into_view(size_t);
      void _repaint_tab(const _InternalTabInfo&);
      void _update_auto_hide_state();
      void _update_layout();
};

#include "./DKTabBarEx.inl"