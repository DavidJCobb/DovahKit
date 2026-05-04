#include "./DKTabBarEx.h"

#pragma region Properties
   void DKTabBarEx::setAutoHide(bool v) {
      if (v == this->autoHide())
         return;
      this->_properties.auto_hide_if_solo_tab = v;
      this->_update_auto_hide_state();
   }

   void DKTabBarEx::setChangeCurrentOnDrag(bool);

   void DKTabBarEx::setDocumentMode(bool);

   void DKTabBarEx::setDrawBase(bool v) {
      if (v == this->drawBase())
         return;
      this->_properties.draw_base = v;
      this->repaint();
   }

   void DKTabBarEx::setElideMode(Qt::TextElideMode);

   void DKTabBarEx::setExpanding(bool);

   void DKTabBarEx::setIconSize(const QSize&);

   void DKTabBarEx::setShape(Shape);

   void DKTabBarEx::setTabsClosable(bool);

   void DKTabBarEx::setUsesScrollButtons(bool);
#pragma endregion
   
#pragma region Tab properties
   QString DKTabBarEx::accessibleTabName(size_t i) const {
      if (auto* info = this->_tab_info(i))
         return info->info.accessibleName;
      return QString();
   }
   void DKTabBarEx::setAccessibleTabName(size_t i, QString v) {
      if (auto* info = this->_tab_info(i))
         info->info.accessibleName = v;
   }

   void DKTabBarEx::setTabButton(size_t i, ButtonPosition pos, QWidget* widget);

   QVariant DKTabBarEx::tabData(size_t i) const {
      if (auto* info = this->_tab_info(i))
         return info->data;
      return {};
   }
   void DKTabBarEx::setTabData(size_t i, const QVariant& v) {
      if (auto* info = this->_tab_info(i))
         info->data = v;
   }

   void DKTabBarEx::setTabEnabled(size_t, bool v);

   QIcon DKTabBarEx::tabIcon(size_t i) const {
      if (auto* info = this->_tab_info(i))
         return info->info.icon;
      return {};
   }
   void DKTabBarEx::setTabIcon(size_t i, QIcon icon) {
      auto* info = this->_tab_info(i);
      if (!info)
         return;
      info->info.icon = icon;
      this->_repaint_tab(*info);
   }

   QString DKTabBarEx::tabText(size_t i) const {
      if (auto* info = this->_tab_info(i))
         return info->info.name;
      return QString();
   }
   void DKTabBarEx::setTabText(size_t i, QString v);

   QColor DKTabBarEx::tabTextColor(size_t i) const {
      if (auto* info = this->_tab_info(i))
         return info->text_color;
      return {};
   }
   void DKTabBarEx::setTabTextColor(size_t i, QColor v) {
      auto* info = this->_tab_info(i);
      if (!info)
         return;
      auto& dst = info->text_color;
      if (dst == v)
         return;
      dst = v;
      this->_repaint_tab(*info);
   }

   QString DKTabBarEx::tabToolTip(size_t i) const {
      if (auto* info = this->_tab_info(i))
         return info->info.toolTip;
      return QString();
   }
   void DKTabBarEx::setTabToolTip(size_t i, QString v);

   void DKTabBarEx::setTabVisible(size_t i, bool v);

   QString DKTabBarEx::tabWhatsThis(size_t i) const {
      if (auto* info = this->_tab_info(i))
         return info->info.whatsThis;
      return QString();
   }
   void DKTabBarEx::setTabWhatsThis(size_t i, QString v);
#pragma endregion

void DKTabBarEx::setCurrentIndex(std::optional<size_t> i);

std::optional<size_t> DKTabBarEx::tabAt(const QPoint& point) const {
   for (size_t i = 0; i < this->_tabs.size(); ++i)
      if (this->_tabs[i]->rect.contains(point))
         return i;
   return {};
}
QRect DKTabBarEx::tabRect(size_t i) const {
   if (auto* info = this->_tab_info(i))
      return info->rect;
   return {};
}

size_t DKTabBarEx::insertTab(size_t at, QIcon icon, QString name);
void DKTabBarEx::moveTab(size_t from, size_t to);
void DKTabBarEx::removeTab(size_t i);


#pragma region Overrides
   /*virtual*/ QSize DKTabBarEx::minimumSizeHint() const /*override*/;
   /*virtual*/ QSize DKTabBarEx::sizeHint() const /*override*/;

   /*virtual*/ void DKTabBarEx::changeEvent(QEvent* event) /*override*/;
   /*virtual*/ bool DKTabBarEx::event(QEvent* event) /*override*/;
   /*virtual*/ void DKTabBarEx::hideEvent(QHideEvent*) /*override*/;
   /*virtual*/ void DKTabBarEx::keyPressEvent(QKeyEvent* event) /*override*/;
   /*virtual*/ void DKTabBarEx::mouseMoveEvent(QMouseEvent* event) /*override*/;
   /*virtual*/ void DKTabBarEx::mousePressEvent(QMouseEvent* event) /*override*/;
   /*virtual*/ void DKTabBarEx::mouseReleaseEvent(QMouseEvent* event) /*override*/;
   /*virtual*/ void DKTabBarEx::paintEvent(QPaintEvent*) /*override*/;
   /*virtual*/ void DKTabBarEx::resizeEvent(QResizeEvent*) /*override*/;
   /*virtual*/ void DKTabBarEx::showEvent(QShowEvent*) /*override*/;
   /*virtual*/ void DKTabBarEx::timerEvent(QTimerEvent* event) /*override*/;
   /*virtual*/ void DKTabBarEx::wheelEvent(QWheelEvent* event) /*override*/;
#pragma endregion

void DKTabBarEx::_force_tab_into_view(size_t);
void DKTabBarEx::_repaint_tab(const _InternalTabInfo& info) {
   this->repaint(info.rect);
}
void DKTabBarEx::_update_auto_hide_state();
void DKTabBarEx::_update_layout();