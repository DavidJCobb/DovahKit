#include "./DKBreadcrumbBar.h"
#include <algorithm> // std::reverse
#include <QApplication>
#include <QCommonStyle>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QStyleOptionButton>

namespace {
   // When drawing a rect with an odd pen (stroke) width, Qt rounds 
   // the stroke down. This means that for a rect with a 1px border, 
   // the left and top borders are inside the rect, and the right and 
   // bottom borders are outside the rect. As you might expect, this 
   // is very silly and we have to adjust for it basically everywhere.
   static constexpr const QMargins qt_border_jank = { 0, 0, 1, 1 };
}

DKBreadcrumbBar::DKBreadcrumbBar(QWidget* parent) : QWidget(parent) {
   this->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);

   {
      auto* textbox = this->_subwidgets.textbox = new QLineEdit(this);
      textbox->setVisible(false);
      textbox->installEventFilter(this);
      QObject::connect(textbox, &QLineEdit::editingFinished, this, [this]() {
         this->_subwidgets.textbox->setVisible(false);
         this->_navigate_to_path(this->_subwidgets.textbox->text());
         this->setFocus();
         this->repaint();
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

bool DKBreadcrumbBar::textEditingAllowed() const noexcept {
   return this->_text_editing.allowed;
}
void DKBreadcrumbBar::setTextEditingAllowed(bool v) {
   if (v == this->_text_editing.allowed)
      return;
   this->_text_editing.allowed = v;
   if (!v) {
      auto* textbox = this->_subwidgets.textbox;
      if (textbox->isVisible()) {
         if (QApplication::focusWidget() == textbox) {
            this->setFocus();
         }
         textbox->setHidden(true);
         this->repaint();
      }
   }
}

QChar DKBreadcrumbBar::textSeparator() const noexcept {
   return this->_text_editing.separator;
}
void DKBreadcrumbBar::setTextSeparator(QChar c) {
   auto& prop = this->_text_editing.separator;
   if (prop == c)
      return;
   prop = c;
   //
   if (this->textEditingAllowed()) {
      auto* textbox = this->_subwidgets.textbox;
      if (textbox->isVisible()) {
         this->_update_textbox_value();
      }
   }
}

Qt::CaseSensitivity DKBreadcrumbBar::caseSensitivity() const noexcept {
   return this->_text_editing.case_sensitivity;
}
void DKBreadcrumbBar::setCaseSensitivity(Qt::CaseSensitivity v) {
   auto& prop = this->_text_editing.case_sensitivity;
   if (prop == v)
      return;
   prop = v;
}

QString DKBreadcrumbBar::path() const noexcept {
   size_t size = 0;
   for (size_t i = 0; i < this->_segments.size(); ++i) {
      if (i > 0)
         ++size;
      size += this->_segments[i].text.size();
   }
   QString path;
   path.reserve(size);
   for (size_t i = 0; i < this->_segments.size(); ++i) {
      if (i > 0)
         path += this->_text_editing.separator;
      path += this->_segments[i].text;
   }
   return path;
}

QMenu* DKBreadcrumbBar::rootMenu() const noexcept {
   return this->_root_button.menu;
}
void DKBreadcrumbBar::setRootMenu(QMenu* m) {
   QMenu* prior = this->_root_button.menu;
   if (m == prior)
      return;
   bool menu_state_changed = false;
   if (this->_state.menu_open_for == index_of_root_button) {
      if (prior)
         prior->setVisible(false);
      this->_state.menu_open_for = index_of_none;
      menu_state_changed = true;
   }
   this->_root_button.menu = m;
   if (!!m != !!prior) {
      this->_re_layout();
   } else if (menu_state_changed) {
      this->repaint();
   }
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
void DKBreadcrumbBar::_re_layout(bool force) {
   auto& last_layout = this->_state.last_layout;
   auto& next_layout = this->_state.next_layout;

   bool guaranteed_relayout = force;
   if (!guaranteed_relayout) {
      if (
         next_layout.segments_changed
      || last_layout.any_truncated
      || last_layout.root_button != (this->_root_button.menu != nullptr)
      ) {
         guaranteed_relayout = true;
      }
   }
   next_layout.segments_changed = {};

   {
      auto* textbox = this->_subwidgets.textbox;
      textbox->setGeometry(0, 0, this->width(), this->height());
      //
      // TODO: Make room for current icon, if visible.
      //
   }

   bool show_root_button = this->_root_button.menu != nullptr;
   {
      int x = this->_styles.border_width;
      int y = this->_styles.border_width;
      int h = this->minimumSizeHint().height();
      h -= this->_styles.border_width * 2; // the widget height includes the widget's own border, so reduce the segment height accordingly
      this->_root_button.geometry = QRect(x, y, this->_styles.segment.menu_button_width, h);
   };

   auto& segments = this->_segments;

   size_t count = segments.size();
   if (!count) {
      last_layout.any_truncated = false;
      last_layout.count_shown   = 0;
      last_layout.root_button   = this->_root_button.menu != nullptr;
      if (guaranteed_relayout) {
         this->repaint();
      }
      return;
   }

   const auto metrics = QFontMetrics(this->font());

   std::vector<int> widths;
   widths.resize(count);
   {
      int x = this->_styles.border_width;
      int y = this->_styles.border_width;
      int h = this->minimumSizeHint().height();
      if (show_root_button) {
         x += this->_root_button.geometry.width();
         --x; // overlap borders
      }
      h -= this->_styles.border_width * 2; // the widget height includes the widget's own border, so reduce the segment height accordingly

      const int mw = this->_styles.segment.margins.left() + this->_styles.segment.margins.right();

      for (size_t i = 0; i < count; ++i) {
         auto& seg = segments[i];
         seg.culled = false;

         auto& main = seg.geometry.main_button;
         auto& menu = seg.geometry.menu_button;
         main = metrics.boundingRect(seg.text);
         main.translate(x, -main.y() + y);
         main.setWidth(main.width() + mw);
         main.setHeight(h);
         x += main.width();
         --x; // overlap borders
         if (seg.menu) {
            menu.setLeft(x);
            menu.setTop(y);
            menu.setWidth(this->_styles.segment.menu_button_width);
            menu.setHeight(h);
            x += menu.width();
            --x; // overlap borders
         } else {
            menu = {};
         }
         widths[i] = main.width() + menu.width() - 1;
      }
   }

   bool   any_truncated = false;
   size_t count_to_show = count;

   int total_width = 0;
   {
      const auto& seg = this->_segments.back();
      if (seg.menu) {
         total_width = seg.geometry.menu_button.right();
      } else {
         total_width = seg.geometry.main_button.right();
      }
   }

   int available = this->width();
   if (available < total_width) {
      if (available < widths.back()) {
         count_to_show = 1;
         widths.back() = available;
         segments.back().geometry.main_button.setWidth(available - this->_styles.segment.menu_button_width);
         segments.back().geometry.menu_button.setX(segments.back().geometry.main_button.right() - 1); // minus 1 to overlap borders
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
         auto& seg = segments[i];
         seg.culled = true;
         if (i == first_to_show - 1) {
            if (seg.menu) {
               dx = seg.geometry.menu_button.right();
            } else {
               dx = seg.geometry.main_button.right();
            }
            --dx; // overlap borders
            --dx; // account for leftmost to-be-shown segment's lefthand border being outside of the rect
         }
      }
      for (; i < count; ++i) {
         auto& seg = segments[i];
         seg.culled = false;

         auto& main = seg.geometry.main_button;
         auto& menu = seg.geometry.menu_button;
         main.translate(-dx, 0);
         menu.translate(-dx, 0);
      }
      if (!show_root_button && count_to_show < count) {
         //
         // Make space to show a "..." indicator where the root button 
         // would ordinarily be.
         //
         show_root_button = true;
         //
         // And bump the segments forward out of the root button's way.
         //
         int dx = this->_root_button.geometry.width();
         --dx; // overlap borders
         available -= dx;
         for (auto& seg : this->_segments) {
            seg.geometry.main_button.translate(dx, 0);
            seg.geometry.menu_button.translate(dx, 0);
         }
      }
   }

   last_layout.any_truncated = any_truncated;
   last_layout.count_shown   = count_to_show;
   last_layout.root_button   = this->_root_button.menu != nullptr;

   this->repaint();
}

void DKBreadcrumbBar::_on_segment_hovered(size_t i) {
   switch (i) {
      case index_of_root_button:
      case index_of_none:
         break;
      default:
         if (i >= this->_segments.size())
            i = index_of_none;
         break;
   }
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
      this->_open_menu(i);
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
   QPoint open_from;
   QMenu* to_open = nullptr;
   switch (i) {
      case index_of_none:
         break;
      case index_of_root_button:
         to_open   = this->_root_button.menu;
         open_from = this->_root_button.geometry.bottomLeft().toPoint();
         break;
      default:
         {
            const auto& seg = this->_segments[i];
            to_open   = this->_segments[i].menu;
            open_from = seg.geometry.menu_button.bottomRight().toPoint();
            open_from.rx() -= 32;
            //
            // The intent of the displacement above is to make it so that opening the root menu 
            // aligns the menu roughly with the leading edge of the widget. The same displacement 
            // is applied to Windows's native breadcrumb widgets for each segment's menu as well, 
            // such that for a segment's menu, icons on menu items are aligned roughly below the 
            // segment text, while the leading edge of the menu items' labels is aligned roughly 
            // below the bottom-right corner of the segment's menu button.
         }
         break;
   }

   if (i == this->_state.menu_open_for) {
      if (to_open && to_open == QApplication::activePopupWidget())
         return;
   }
   this->_close_menu(this->_state.menu_open_for);

   if (!to_open) {
      //
      // We want to forcibly close any menu that's already open on a segment, even if 
      // we don't open a menu ourselves. This makes it simpler and easier to maintain 
      // consistent behaviors with Windows's native breadcrumb menu. (Specifically, 
      // the native widget tracks whether its menu is "supposed to be" open. If you 
      // open a segment's menu and then use the left and right arrow keys to hover a 
      // segment with no menu, then no menu will be visible; but pressing left or 
      // right *again* and moving to a segment that has a menu will pop that segment's 
      // menu.)
      //
      this->_state.menu_open_for = i;
      return;
   }

   if (i == index_of_root_button) {
      //
      // The root menu doesn't know its own index, and won't tell us when it's opened, 
      // so we here need to manually track that it's the menu we have open.
      //
      this->_state.menu_open_for = index_of_root_button;
   }
   to_open->popup(this->mapToGlobal(open_from));
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
   const size_t count = this->_segments.size();
   if (!count) {
      if (this->_root_button.menu) {
         this->_on_segment_hovered(index_of_root_button);
      }
      return;
   }
   const size_t first = [&]() -> size_t {
      if (this->_root_button.menu) {
         return index_of_root_button;
      }
      const auto vis = this->_state.last_layout.count_shown;
      if (count >= vis) {
         return count - vis;
      }
      return 0;
   }();

   size_t i = this->_state.hovered_segment;
   if (i == index_of_none) {
      if (left) {
         i = count - 1;
      } else {
         i = first;
      }
   } else if (i == index_of_root_button) {
      if (left)
         i = count - 1;
      else
         i = 0;
   } else {
      if (left) {
         i = ((i == first) ? count : i) - 1;
      } else {
         if (++i >= count)
            i = first;
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

void DKBreadcrumbBar::_update_textbox_value() {
   auto*      textbox = this->_subwidgets.textbox;
   const auto blocker = QSignalBlocker(textbox);
   textbox->setText(this->path());
}
bool DKBreadcrumbBar::_navigate_to_path(QString path) {
   if (!this->_data.model)
      return false;
   const auto cs     = this->caseSensitivity();
   const auto chunks = path.splitRef(this->_text_editing.separator, Qt::SkipEmptyParts, cs);
   //
   // Try to see if this matches a subset of the path we're already in. 
   // If so, that saves us some model queries.
   //
   QModelIndex qmi;
   size_t ci = 0; // chunk index
   {
      size_t max = std::min((size_t)chunks.size(), this->_segments.size());
      for (; ci < max; ++ci) {
         auto& seg = this->_segments[ci];
         if (seg.text.compare(chunks[ci], cs) != 0)
            break;
         qmi = seg.qmi;
      }
   }
   for (; ci < chunks.size(); ++ci) {
      auto rows = this->_data.model->rowCount(qmi);
      if (rows <= 0)
         return false; // failed.
      bool found = false;
      for (size_t ri = 0; ri < rows; ++ri) {
         auto row_qmi = this->_data.model->index(ri, 0, qmi);
         if (!row_qmi.isValid())
            continue;
         auto text = this->_data.model->data(row_qmi, Qt::DisplayRole).toString();
         if (text.isEmpty())
            continue;
         if (text.compare(chunks[ci], cs) == 0) {
            qmi   = row_qmi;
            found = true;
            break;
         }
      }
      if (!found)
         return false;
   }
   this->setCurrentIndex(qmi);
   return true;
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
   h += 2; // segment borders
   {
      auto metrics = QFontMetrics(this->font());
      h += metrics.height();
   }
   h += this->_styles.border_width * 2; // widget borders
   size.setHeight(h);

   return size;
}
/*virtual*/ QSize DKBreadcrumbBar::sizeHint() const /*override*/ {
   return this->minimumSizeHint();
}
//
#pragma region Events
   /*virtual*/ void DKBreadcrumbBar::changeEvent(QEvent* event) {
      switch (event->type()) {
         case QEvent::EnabledChange:
            if (this->_subwidgets.textbox->isVisible()) {
               this->_subwidgets.textbox->setVisible(false);
            }
            {
               size_t i = this->_state.menu_open_for;
               if (i < this->_segments.size()) {
                  auto& seg = this->_segments[i];
                  if (seg.menu)
                     seg.menu->setVisible(false);
               } else if (i == index_of_root_button) {
                  if (QMenu* menu = this->_root_button.menu)
                     menu->setVisible(false);
               }
               this->_state.menu_open_for = index_of_none;
            }
            this->repaint();
            break;
         case QEvent::LayoutDirectionChange:
            this->_re_layout(true);
            break;
      }
   }
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

         if (this->_root_button.menu) {
            if (this->_root_button.geometry.contains(pos)) {
               this->_open_menu(index_of_root_button);
               return;
            }
         }
         for (size_t i = 0; i < this->_segments.size(); ++i) {
            const auto& segment = this->_segments[i];
            if (segment.culled)
               continue;
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
         if (this->textEditingAllowed()) {
            this->_update_textbox_value();
            this->_subwidgets.textbox->setVisible(true);
            this->_subwidgets.textbox->setFocus();
            this->_subwidgets.textbox->selectAll();
            event->accept();
         }
         return;
      }
   }
   /*virtual*/ void DKBreadcrumbBar::paintEvent(QPaintEvent* event) {
      QPainter painter(this);

      if (this->_subwidgets.textbox->isVisible())
         return;

      const bool is_disabled  = !this->isEnabled();
      const bool is_menu_open = this->_state.menu_open_for != index_of_none;

      // Widget base layer
      {
         painter.setPen(QPen(this->palette().color(QPalette::Dark), this->_styles.border_width));
         painter.setBrush(this->palette().color(QPalette::Base));
         auto rect = QRect(0, 0, this->width(), this->height()) - qt_border_jank;
         painter.drawRect(rect);
      }

      //
      // TODO: Show icon for current index, to the left of the root
      // NOTE: For RTL layouts, it should probably be to the right instead
      //

      QPainterPath menu_chevron_elided;
      QPainterPath menu_chevron_base;
      QPainterPath menu_chevron_open;
      {
         qreal cx = this->_styles.segment.menu_button_width / 2.0F;
         qreal cy = this->height() / 2.0F;

         menu_chevron_open.moveTo(QPointF{ cx - 2, cy - 1 });
         menu_chevron_open.lineTo(QPointF{ cx,     cy + 1 });
         menu_chevron_open.lineTo(QPointF{ cx + 2, cy - 1 });

         bool is_rtl = false;
         if (auto* app = qobject_cast<QGuiApplication*>(QApplication::instance())) {
            is_rtl = app->layoutDirection() == Qt::LayoutDirection::RightToLeft;
         }
         int pos = is_rtl ? -1 : 1;
         menu_chevron_base.moveTo(QPointF{ cx - pos, cy - 2 });
         menu_chevron_base.lineTo(QPointF{ cx + pos, cy });
         menu_chevron_base.lineTo(QPointF{ cx - pos, cy + 2 });

         menu_chevron_elided.moveTo(QPointF{ cx-(pos) - pos, cy - 2});
         menu_chevron_elided.lineTo(QPointF{ cx-(pos) + pos, cy });
         menu_chevron_elided.lineTo(QPointF{ cx-(pos) - pos, cy + 2 });
         menu_chevron_elided.moveTo(QPointF{ cx+(pos) - pos, cy - 2 });
         menu_chevron_elided.lineTo(QPointF{ cx+(pos) + pos, cy });
         menu_chevron_elided.lineTo(QPointF{ cx+(pos) - pos, cy + 2 });
      }
      const auto* active_popup = QApplication::activePopupWidget();

      const auto& _colors_for_segment = [this, is_disabled, is_menu_open](size_t i) -> SegmentPalette& {
         const bool  hovered = !is_menu_open && this->_state.hovered_segment == i;
         return
            is_disabled ?
               this->_styles.segment.colors.disabled
            :
               hovered ?
                  this->_styles.segment.colors.hovered
               :
                  this->_styles.segment.colors.normal
         ;
      };
      auto _draw_icon = [&painter](const SegmentPalette& colors, QPointF pos, const QPainterPath& path) {
         painter.save();
         painter.setPen(colors.menu_button.icon);
         painter.translate(pos);
         painter.setRenderHint(QPainter::RenderHint::Antialiasing);
         painter.drawPath(path);
         painter.restore();
      };

      if (this->_root_button.menu) {
         const auto& colors = _colors_for_segment(index_of_root_button);
         painter.setBrush(colors.menu_button.fill);
         painter.setPen(colors.menu_button.line);
         painter.drawRect(this->_root_button.geometry - qt_border_jank);
         const auto& icon =
            (this->_state.last_layout.count_shown < this->_segments.size()) ?
               menu_chevron_elided
            :
               (active_popup == this->_root_button.menu) ?
                  menu_chevron_open
               :
                  menu_chevron_base
         ;
         _draw_icon(colors, this->_root_button.geometry.topLeft(), icon);
      } else if (this->_state.last_layout.count_shown < this->_segments.size()) {
         //
         // Draw a "..." button to indicate that not all segments are shown.
         // 
         // QPainterPath doesn't have `drawPoint`, and if QPainter is told to 
         // draw a line from a point to the same point, it produces no output 
         // rather than a single point; so we can't prepare a path for this in 
         // advance.
         //
         qreal cx = this->_styles.segment.menu_button_width / 2.0F;
         qreal cy = this->height() / 2.0F;
         painter.save();
         painter.setPen(this->palette().color(QPalette::Text));
         painter.setRenderHint(QPainter::RenderHint::Antialiasing);
         painter.translate(this->_root_button.geometry.topLeft());
         painter.drawPoint(QPointF{ cx - 2, cy });
         painter.drawPoint(QPointF{ cx    , cy });
         painter.drawPoint(QPointF{ cx + 2, cy });
         painter.restore();
      }

      painter.setFont(this->font());
      for (size_t i = 0; i < this->_segments.size(); ++i) {
         const auto& segment = this->_segments[i];
         if (segment.culled) {
            continue;
         }
         const auto& colors = _colors_for_segment(i);
         if (segment.menu) {
            painter.setBrush(colors.menu_button.fill);
            painter.setPen(colors.menu_button.line);
            painter.drawRect(segment.geometry.menu_button - qt_border_jank);
            _draw_icon(
               colors,
               segment.geometry.menu_button.topLeft(),
               (active_popup == segment.menu) ?
                  menu_chevron_open
               :
                  menu_chevron_base
            );
         }
         if (!segment.geometry.main_button.isEmpty()) {
            painter.setBrush(colors.main_button.fill);
            painter.setPen(colors.main_button.line);
            painter.drawRect(segment.geometry.main_button - qt_border_jank);
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
         if (event->type() == QEvent::KeyPress) {
            auto* kev = (QKeyEvent*)event;
            if (kev->key() == Qt::Key::Key_Escape) {
               this->_subwidgets.textbox->setVisible(false);
               this->setFocus();
               this->repaint();
            }
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
      return QWidget::eventFilter(watched, event);
   }
#pragma endregion