#include "./DKBreadcrumbBar.h"
#include <algorithm> // std::reverse
#include <QApplication>
#include <QCommonStyle>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QStyleOptionButton>

DKBreadcrumbBar::DKBreadcrumbBar(QWidget* parent) : QFrame(parent) {
   this->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
   this->setMinimumWidth(100);
   //
   this->setFrameStyle(QFrame::Shape::Box | QFrame::Shadow::Sunken);

   {
      auto* textbox = this->_subwidgets.textbox = new QLineEdit(this);
      textbox->setVisible(false);
      textbox->installEventFilter(this);
      QObject::connect(textbox, &QLineEdit::editingFinished, this, [this]() {
         this->_subwidgets.textbox->setVisible(false);
         //
         // TODO: Apply the edit.
         //
      });
   }

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setMouseTracking(true);
}

QAbstractItemModel* DKBreadcrumbBar::model() const noexcept {
   return this->_data.model;
}
void DKBreadcrumbBar::setModel(QAbstractItemModel* model) {
   auto* prior = this->model();
   if (prior == model)
      return;
   if (prior) {
      QObject::disconnect(prior, nullptr, this, nullptr);
   }
   this->_data.model = model;
   this->_data.index = {};
   if (model) {
      QObject::connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, [this, model](const QModelIndex& parent, int first, int last) {
         for (int i = first; i <= last; ++i) {
            auto qmi = model->index(i, 0, parent);
            if (this->_on_before_item_deleted(qmi))
               break;
         }
      });
   }
   this->_on_navigated();
}

QModelIndex DKBreadcrumbBar::currentIndex() const noexcept {
   return this->_data.index;
}
void DKBreadcrumbBar::setCurrentIndex(const QModelIndex& qmi) {
   if (this->_data.index == qmi)
      return;
   if (qmi.model() != this->model())
      return;
   this->_data.index = qmi;
   this->_on_navigated();
}

bool DKBreadcrumbBar::allowTextEditing() const noexcept {
   return true;
}

void DKBreadcrumbBar::_on_navigated() {
   //
   // Teardown.
   //
   this->setFocusProxy(nullptr);
   {
      auto& list = this->_segments;
      for (auto& seg : list) {
         if (seg.menu) {
            seg.menu->setVisible(false);
            seg.menu->deleteLater();
            seg.menu = nullptr;
         }
      }
      list.clear();
   }
   this->_state.hovered_segment = index_of_none;
   this->_state.menu_open_for   = index_of_none;
   //
   // Rebuild.
   //
   {
      auto* model = this->model();
      auto& list  = this->_segments;
      auto  qmi   = this->currentIndex();
      bool  basis = true;
      do {
         auto    flags = model->flags(qmi);
         QString label = model->data(qmi, Qt::DisplayRole).toString();
         if (label.isEmpty() && !qmi.isValid()) {
            break;
         }
         auto& seg = this->_segments.emplace_back();
         seg.qmi  = qmi;
         seg.text = label;
         if (!basis) { // don't show a menu on the current-index item
            if (!(flags & Qt::ItemNeverHasChildren) && model->rowCount(qmi) > 0) {
               auto* menu = seg.menu = new QMenu(this);
               QObject::connect(menu, &QMenu::aboutToShow, this, [this, menu, qmi]() {
                  this->_state.menu_open_for = menu->property("segment-index").toInt();
                  this->_set_up_menu(*menu, qmi);
               });
               QObject::connect(menu, &QMenu::aboutToHide, this, &DKBreadcrumbBar::_on_segment_menu_hidden);
               this->_start_menu_eavesdropping(*menu);
            }
         }
      } while (qmi = model->parent(qmi), basis = false, qmi.isValid());
      std::reverse(list.begin(), list.end());
      //
      // Tell each segment's menu what its index (as in list position, not QMI) 
      // is, to simplify signal handling.
      //
      if (list.size() > 0) {
         for (size_t i = 0; i < list.size() - 1; ++i) { // minus one because the current-item (i.e. last segment) never has a menu
            auto& seg = this->_segments[i];
            if (seg.menu)
               seg.menu->setProperty("segment-index", i);
         }
      }
   }
   this->_state.hovered_segment = index_of_none;
   this->_state.next_layout.segments_changed = true;
   this->_re_layout();

   emit this->currentIndexChanged(this->currentIndex());
}
void DKBreadcrumbBar::_on_data_changed(const QModelIndex& qmi) {
   auto& list  = this->_segments;
   bool  found = false;
   for (size_t i = 0; i < list.size(); ++i) {
      auto& seg = list[i];
      if (seg.qmi == qmi) {
         QString label = this->model()->data(seg.qmi, Qt::DisplayRole).toString();
         seg.text = label;
         found = true;
         break;
      }
   }
   if (found) {
      this->_state.next_layout.segments_changed = true;
      this->_re_layout();
   }
}
bool DKBreadcrumbBar::_on_before_item_deleted(const QModelIndex& qmi) {
   QModelIndex parent;
   
   auto& list  = this->_segments;
   bool  found = false;
   for (size_t i = 0; i < list.size(); ++i) {
      auto& seg = list[i];
      if (seg.qmi == qmi) {
         found = true;
         if (i > 0) {
            parent = list[i - 1].qmi;
         }
         break;
      }
   }
   if (!found)
      return false;
   this->setCurrentIndex(parent);
   return true;
}
void DKBreadcrumbBar::_set_up_menu(QMenu& menu, const QModelIndex& qmi) {
   menu.clear();

   QModelIndex selected_child;
   {
      auto& list = this->_segments;
      for (size_t i = 0; i < list.size(); ++i){
         if (qmi == list[i].qmi) {
            if (i + 1 < list.size()) {
               selected_child = list[i + 1].qmi;
            }
            break;
         }
      }
   }

   auto* model = this->model();
   if (!model)
      return;
   size_t rows = model->rowCount(qmi);
   for (size_t i = 0; i < rows; ++i) {
      const auto child_qmi = model->index(i, 0, qmi);
      QString    label     = model->data(child_qmi, Qt::DisplayRole).toString();
      QVariant   icon      = model->data(child_qmi, Qt::DecorationRole);
      if (label.isEmpty())
         continue;

      QAction* action = new QAction(label, &menu);
      if (icon.canConvert<QIcon>()) {
         action->setIcon(icon.value<QIcon>());
      }
      if (selected_child.isValid() && child_qmi == selected_child) {
         auto font = action->font();
         font.setBold(true);
         action->setFont(font);
      }

      QPersistentModelIndex qpmi = child_qmi;
      QObject::connect(action, &QAction::triggered, this, [this, qpmi]() {
         this->setCurrentIndex(qpmi);
      });
      menu.addAction(action);
   }
}
void DKBreadcrumbBar::_re_layout() {
   auto& last_layout = this->_state.last_layout;
   auto& next_layout = this->_state.next_layout;

   bool guaranteed_relayout = false;
   if (next_layout.segments_changed || last_layout.any_truncated) {
      guaranteed_relayout = true;
   }
   next_layout.segments_changed = {};

   auto& segments = this->_segments;

   auto*  style = this->style();
   size_t count = segments.size();
   if (!count) {
      last_layout.any_truncated = false;
      last_layout.count_shown   = 0;
      return;
   }

   {
      auto* textbox = this->_subwidgets.textbox;
      textbox->setGeometry(0, 0, this->width(), this->height());
      //
      // TODO: Make room for root icon, etc.
      //
   }

   const auto metrics = QFontMetrics(this->font());

   int total_width = 0;
   std::vector<int> widths;
   widths.resize(count);
   {
      int x = 0;
      int y = 0;
      int h = this->minimumSizeHint().height();

      const int mw = this->_styles.segment.margins.left() + this->_styles.segment.margins.right();

      for (size_t i = 0; i < count; ++i) {
         auto& seg = segments[i];
         seg.culled = false;

         auto& main = seg.geometry.main_button;
         auto& menu = seg.geometry.menu_button;
         main = metrics.boundingRect(seg.text);
         main.setWidth(main.width());
         main.translate(x, -main.y() + y);
         main.setWidth(main.width() + mw);
         main.setHeight(h);
         x += main.width();
         if (seg.menu) {
            menu.setLeft(x);
            menu.setWidth(this->_styles.segment.menu_button_width);
            menu.setHeight(h);
            x += menu.width();
         } else {
            menu = {};
         }
         widths[i] = main.width() + menu.width();
         total_width += widths[i];
      }
   }

   bool   any_truncated = false;
   size_t count_to_show = count;

   int available = this->size().width();
   if (available < total_width) {
      if (available < widths.back()) {
         count_to_show = 1;
         widths.back() = available;
         segments.back().geometry.main_button.setWidth(available - this->_styles.segment.menu_button_width);
         segments.back().geometry.menu_button.setX(segments.back().geometry.main_button.right());
      } else {
         count_to_show = 0;
         total_width   = 0;
         for (count_to_show = 0; count_to_show < count; ++count_to_show) {
            size_t i = count - count_to_show - 1;
            if (total_width + widths[i] > available)
               break;
            total_width += widths[i];
         }
      }
   }
   if (!guaranteed_relayout) {
      if (!any_truncated && last_layout.count_shown == count_to_show)
         return;
   }

   size_t first_to_show = count - count_to_show;
   {
      int    dx = 0;
      size_t i  = 0;
      for (; i < first_to_show; ++i) {
         segments[i].culled = true;
         dx += widths[i];
      }
      int x = this->contentsMargins().left();
      int y = this->contentsMargins().top();
      for (; i < count; ++i) {
         auto& seg = segments[i];
         seg.culled = false;

         auto& main = seg.geometry.main_button;
         auto& menu = seg.geometry.menu_button;
         main.translate(-dx, 0);
         menu.translate(-dx, 0);
      }
   }

   last_layout.any_truncated = any_truncated;
   last_layout.count_shown   = count_to_show;

   this->repaint();
}

void DKBreadcrumbBar::_on_segment_hovered(size_t i) {
   if (i >= this->_segments.size())
      i = index_of_none;
   if (this->_state.hovered_segment == i)
      return;

   this->_state.hovered_segment = i;
   this->repaint();
   //
   // If a segment's menu is open but the mouse moves to another 
   // segment, then close the former segment's menu and open the 
   // latter segment's menu.
   //
   if (i != index_of_none && this->_state.menu_open_for != index_of_none) {
      this->_open_menu(this->_state.hovered_segment);
   }
}

void DKBreadcrumbBar::_close_menu(size_t i) {
   if (i == index_of_none || i >= this->_segments.size())
      return;
   auto& seg = this->_segments[i];
   if (!seg.menu)
      return;
   seg.menu->hide();
}
void DKBreadcrumbBar::_open_menu(size_t i) {
   const segment& seg = this->_segments[i];
   if (i == this->_state.menu_open_for) {
      if (seg.menu && seg.menu == QApplication::activePopupWidget())
         return;
   }

   this->_close_menu(this->_state.menu_open_for);
   if (!seg.menu) {
      //
      // We want to forcibly close any menu that's already open on a segment, even if 
      // we don't open a menu ourselves. This makes it simpler and easier to maintain 
      // consistent behaviors with Windows's native breadcrumb menu.
      //
      this->_state.menu_open_for = i;
      return;
   }

   auto pos = seg.geometry.menu_button.bottomRight().toPoint();
   pos.rx() -= 32; // icon size
   pos = this->mapToGlobal(pos);
   //
   // The intent of the displacement above is to make it so that opening the root menu 
   // aligns the menu roughly with the leading edge of the widget. The same displacement 
   // is applied to Windows's native breadcrumb widgets for each segment's menu as well, 
   // such that for a segment's menu, icons on menu items are aligned roughly below the 
   // segment text, while the leading edge of the menu items' labels is aligned roughly 
   // below the bottom-right corner of the segment's menu button.

   seg.menu->popup(pos);
   this->repaint(); // to update the menu-button chevron for each segment
}
void DKBreadcrumbBar::_start_menu_eavesdropping(QMenu& menu) {
   //
   // When a "popup" widget, such as a QMenu, is open, Qt enters "popup mode." 
   // This causes QApplication to deliver events differently: all mouse, touch, 
   // and keyboard events are re-routed to the menu.
   // 
   // This is actually kind of important: QMenu has some accommodations which 
   // make it so that if you move the mouse diagonally from an item to a submenu, 
   // the menu won't instantly close as the mouse cuts that corner. But we want 
   // to still receive those mouse events ourselves (e.g. as a QMenuBar would), 
   // so we need to install ourselves as an event filter on the menu.
   //
   menu.installEventFilter(this);
}
bool DKBreadcrumbBar::_do_menu_eavesdropping(QMenu& menu, QEvent& event) {
   if (QApplication::activePopupWidget() != &menu)
      return false;
   //
   // Peek at application-wide events that are being re-routed to our 
   // open menu.
   //
   switch (event.type()) {
      case QEvent::MouseMove:
         this->mouseMoveEvent((QMouseEvent*)&event);
         break;
      case QEvent::MouseButtonPress:
         /*{
            auto* mev = (QMouseEvent*)&event;
            if (this == QApplication::widgetAt(mev->globalPos())) {
               this->mousePressEvent(mev);
               return true;
            }
         }*/
         break;
      case QEvent::KeyPress:
         //
         // We only care about keypresses that don't occur while the 
         // cursor is over the menu.
         //
         if (&menu != QApplication::widgetAt(QCursor::pos())) {
            this->keyPressEvent((QKeyEvent*)&event);
         }
         break;
   }
   return false;
}
void DKBreadcrumbBar::_on_segment_menu_hidden() {
   this->_state.menu_open_for = index_of_none;
}

void DKBreadcrumbBar::_on_segment_clicked(const segment& seg) {
   this->_close_menu(this->_state.menu_open_for);
   this->setCurrentIndex(seg.qmi);
}
void DKBreadcrumbBar::_on_horizontal_arrow_key(bool left) {
   size_t i = this->_state.hovered_segment;
   if (this->_segments.empty())
      return;
   if (i == index_of_none) {
      if (left) {
         i = this->_segments.size() - 1;
      } else {
         i = 0;
      }
   } else {
      if (left) {
         i = ((i == 0) ? this->_segments.size() : i) - 1;
      } else {
         if (++i >= this->_segments.size())
            i = 0;
      }
   }
   this->_on_segment_hovered(i);
}
void DKBreadcrumbBar::_on_vertical_arrow_key() {
   auto& segments  = this->_segments;
   auto  hover_idx = this->_state.hovered_segment;
   if (segments.empty())
      return;
   if (hover_idx == index_of_none) {
      //
      // If there is no "hovered" segment (i.e. because we're hovering 
      // the empty gutter after all segments), then hover the root 
      // button.
      //
      this->_on_segment_hovered(0);
      this->repaint();
      return;
   }
   //
   // TODO: The intended behavior is as follows:
   //
   //  - Pressing the arrow key opens the menu but does not focus it; 
   //    a horizontal arrow key will still navigate within the widget.
   // 
   //  - Pressing a vertical arrow key once the menu has been opened 
   //    will move focus to the menu, allowing keyboard navigation 
   //    within it. At this point, the left and right arrow keys will 
   //    no longer navigate within the widget (i.e. they don't count 
   //    as hovering an adjacent segment) until the menu is closed.
   // 
   // How do we implement this in Qt, especially given our need for 
   // the menu-eavesdropping hack?
   //
   this->_open_menu(hover_idx);
}

/*virtual*/ QSize DKBreadcrumbBar::minimumSizeHint() const /*override*/ {
   auto size = this->minimumSize();

   this->ensurePolished();
   int h = 0;
   {
      const auto  w_margins = this->contentsMargins();
      const auto& s_margins = this->_styles.segment.margins;
      h += w_margins.top() + w_margins.bottom();
      h += s_margins.top() + s_margins.bottom();
   }
   {
      auto metrics = QFontMetrics(this->font());
      h += metrics.height();
   }
   size.setHeight(h);

   return size;
}
/*virtual*/ QSize DKBreadcrumbBar::sizeHint() const /*override*/ {
   return this->minimumSizeHint();
}
//
#pragma region Events
   /*virtual*/ void DKBreadcrumbBar::keyPressEvent(QKeyEvent* event) {
      switch (event->key()) {
         case Qt::Key::Key_Up:
            [[fallthrough]];
         case Qt::Key::Key_Down:
            this->_on_vertical_arrow_key();
            break;
         case Qt::Key::Key_Left:
            this->_on_horizontal_arrow_key(true);
            break;
         case Qt::Key::Key_Right:
            this->_on_horizontal_arrow_key(false);
            break;
      }
   }
   /*virtual*/ void DKBreadcrumbBar::mouseMoveEvent(QMouseEvent* event) {
      //auto pos = event->localPos();
      auto pos = this->mapFromGlobal(event->globalPos());
      // The event may have been originally delivered to a QMenu before we took it, 
      // so we can't trust its own localPos(). Refer to the "menu eavesdropping" 
      // code and its comments for further information.

      for (size_t i = 0; i < this->_segments.size(); ++i) {
         const auto& seg = this->_segments[i];
         if (seg.geometry.main_button.contains(pos) || seg.geometry.menu_button.contains(pos)) {
            this->_on_segment_hovered(i);
            return;
         }
      }
      this->_on_segment_hovered(index_of_none);
   }
   /*virtual*/ void DKBreadcrumbBar::mousePressEvent(QMouseEvent* event) {
      QWidget::mousePressEvent(event);
      if (event->button() != Qt::LeftButton) {
         return;
      }
      if (event->button() == Qt::LeftButton) {
         //auto pos = event->localPos();
         auto pos = this->mapFromGlobal(event->globalPos());
         // The event may have been originally delivered to a QMenu before we took it, 
         // so we can't trust its own localPos(). Refer to the "menu eavesdropping" 
         // code and its comments for further information.

         for (size_t i = 0; i < this->_segments.size(); ++i) {
            const auto& segment = this->_segments[i];
            if (segment.geometry.main_button.contains(pos)) {
               this->_on_segment_clicked(segment);
               event->accept();
               return;
            }
            if (segment.geometry.menu_button.contains(pos)) {
               if (this->_state.menu_open_for == i) {
                  this->_close_menu(i);
                  return;
               }
               this->_open_menu(i);
               return;
            }
         }
         this->_subwidgets.textbox->setVisible(true);
         this->_subwidgets.textbox->setFocus();
         event->accept();
         return;
      }
   }
   /*virtual*/ void DKBreadcrumbBar::paintEvent(QPaintEvent* event) {
      QPainter painter(this);

      if (this->_subwidgets.textbox->isVisible())
         return;

      //
      // TODO: Show icon for current index, to the left of the root
      // NOTE: For RTL layouts, it should probably be to the right instead
      //

      QPainterPath menu_chevron_base;
      QPainterPath menu_chevron_open;
      {
         int cx = this->_styles.segment.menu_button_width / 2;
         int cy = this->height() / 2;

         menu_chevron_open.moveTo(QPoint{ cx - 2, cy - 1 });
         menu_chevron_open.lineTo(QPoint{ cx,     cy + 1 });
         menu_chevron_open.lineTo(QPoint{ cx + 2, cy - 1 });

         // TODO: This is an LTR icon; we need logic for RTL
         menu_chevron_base.moveTo(QPoint{ cx - 1, cy - 2 });
         menu_chevron_base.lineTo(QPoint{ cx + 1, cy });
         menu_chevron_base.moveTo(QPoint{ cx - 1, cy + 2 });
      }

      const bool is_disabled  = !this->isEnabled();
      const bool is_menu_open = this->_state.menu_open_for != index_of_none;

      painter.setFont(this->font());
      for (size_t i = 0; i < this->_segments.size(); ++i) {
         const auto& segment = this->_segments[i];
         if (segment.culled) {
            continue;
         }
         const bool  hovered = i == this->_state.hovered_segment;
         const auto& colors  =
            is_disabled ?
               this->_styles.segment.colors.disabled
            :
               hovered ?
                  this->_styles.segment.colors.hovered
               :
                  this->_styles.segment.colors.normal
         ;
         if (segment.menu) {
            painter.setBrush(colors.menu_button.fill);
            painter.setPen(colors.menu_button.line);
            painter.drawRect(segment.geometry.menu_button);
            {  // Draw menu button glyph
               painter.save();
               painter.setPen(colors.menu_button.icon);
               painter.translate(segment.geometry.menu_button.topLeft());
               if (QApplication::activePopupWidget() == segment.menu) {
                  painter.drawPath(menu_chevron_open);
               } else {
                  painter.drawPath(menu_chevron_base);
               }
               painter.drawPath(menu_chevron_base);
               painter.restore();
            }
         }
         if (!segment.geometry.main_button.isEmpty()) {
            painter.setBrush(colors.main_button.fill);
            painter.setPen(colors.main_button.line);
            painter.drawRect(segment.geometry.main_button);
            //
            auto rect = segment.geometry.main_button;
            rect -= this->_styles.segment.margins;
            painter.setPen(colors.main_button.text);
            painter.drawText(
               rect,
               Qt::AlignLeft,
               segment.text,
               nullptr
            );
         }
      }
   }
   /*virtual*/ void DKBreadcrumbBar::resizeEvent(QResizeEvent* event) {
      this->_re_layout();
   }
   /*virtual*/ void DKBreadcrumbBar::showEvent(QShowEvent* event) {
      this->_re_layout();
   }

   /*virtual*/ bool DKBreadcrumbBar::eventFilter(QObject* watched, QEvent* event) {
      if (watched == this->_subwidgets.textbox) {
         if (event->type() == QEvent::FocusOut) {
            this->_subwidgets.textbox->setVisible(false);
            this->repaint();
         }
      }
      if (auto* casted = qobject_cast<QMenu*>(watched)) {
         if (this->_do_menu_eavesdropping(*casted, *event))
            //
            // Clicking on a segment while a menu is open will cause us to close 
            // the menu. Let's make sure that QMenu doesn't then forward the same 
            // mouse event back to us afterward.
            //
            return true;
      }
      return QFrame::eventFilter(watched, event);
   }
#pragma endregion