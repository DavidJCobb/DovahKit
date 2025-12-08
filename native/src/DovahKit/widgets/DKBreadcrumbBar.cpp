#include "./DKBreadcrumbBar.h"
#include <algorithm> // std::reverse
#include <QApplication>
#include <QCommonStyle>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QProxyStyle>
#include <QStyle>

namespace {
   // When drawing a rect with an odd pen (stroke) width, Qt rounds 
   // the stroke down. This means that for a rect with a 1px border, 
   // the left and top borders are inside the rect, and the right and 
   // bottom borders are outside the rect. As you might expect, this 
   // is very silly and we have to adjust for it basically everywhere.
   static constexpr const QMargins qt_border_jank = { 0, 0, 1, 1 };
}

#pragma region DKBreadcrumbBar::segment
   unsigned int DKBreadcrumbBar::segment::width() const noexcept {
      int width = this->geometry.main_button.width();
      if (this->menu)
         width += this->geometry.menu_button.width();
      if (this->geometry.main_borders.leading)
         ++width;
      if (this->geometry.main_borders.trailing)
         ++width;
      return width;
   }
   void DKBreadcrumbBar::segment::repaint(DKBreadcrumbBar& widget, QPainter& painter, segment_paint_state state, size_t my_index) const {
      auto& palette = [state, &widget]() -> const SegmentPalette& {
         switch (state) {
            case segment_paint_state::disabled:
               return widget._styles.segment.colors.disabled;
            case segment_paint_state::hovered:
               return widget._styles.segment.colors.hovered;
         }
         return widget._styles.segment.colors.normal;
      }();

      if (this->menu) {
         painter.setBrush(palette.menu_button.fill);
         painter.setPen(palette.menu_button.line);
         painter.drawRect(this->geometry.menu_button - qt_border_jank);
         //
         // Draw icon.
         //
         painter.save();
         painter.setPen(palette.menu_button.icon);
         painter.translate(this->geometry.menu_button.center());
         painter.setRenderHint(QPainter::RenderHint::Antialiasing);
         if (my_index == widget._state.menu_open_for) {
            painter.drawPath(widget._state.icons.chevron_open);
         } else {
            painter.drawPath(widget._state.icons.chevron_base);
         }
         painter.restore();
      }
      {
         //
         // Qt's border jank is hard to properly account for and adjust for 
         // throughout the codebase, so some of our calculations end up being 
         // completely correct for hit testing but off for rendering, or off 
         // for both, and it's just a massive headache to try and figure out 
         // why. Easier to just apply spot corrections during rendering.
         //
         constexpr const bool janky_corrections = true;

         painter.fillRect(this->geometry.main_button, palette.main_button.fill);
         //
         // Borders:
         //
         bool draw_left  = this->geometry.main_borders.leading;
         bool draw_right = this->geometry.main_borders.trailing;
         auto rect       = this->geometry.main_button - qt_border_jank;
         if (widget.layoutDirection() == Qt::LayoutDirection::RightToLeft) {
            std::swap(draw_left, draw_right);
            if constexpr (janky_corrections) {
               if (draw_left) {
                  rect.moveLeft(rect.x() - 1);
               }
            }
         }
         if constexpr (janky_corrections) {
            if (this->geometry.main_borders.trailing) {
               rect.setWidth(rect.width() + 1);
            }
         }
         QPainterPath path;
         path.moveTo(rect.topLeft());
         path.lineTo(rect.topRight());
         if (draw_right) {
            path.lineTo(rect.bottomRight());
         } else {
            path.moveTo(rect.bottomRight());
         }
         path.lineTo(rect.bottomLeft());
         if (draw_left) {
            path.lineTo(rect.topLeft());
         }
         painter.strokePath(path, palette.main_button.line);
      }
      painter.setPen(palette.main_button.text);
      painter.drawText(
         this->geometry.main_button - widget._styles.segment.margins,
         Qt::AlignLeading,
         this->text,
         nullptr
      );
   }
#pragma endregion

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

   this->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
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
   if (m) {
      for (const auto& seg : this->_segments) {
         //
         // Theoretically, someone could grab our per-segment menus via 
         // QApplication::activePopupWidget, etc., and try to set one 
         // of those as the root menu. This would be weird and stupid, 
         // but I'll guard against it anyway.
         //
         assert(m != seg.menu && "What the hell are you doing?! Don't take a DKBreadcrumbBar's internal menus and set them as the root menu!");
      }
   }
   bool menu_state_changed = false;
   if (this->_state.menu_open_for == index_of_root_button) {
      if (prior)
         prior->setVisible(false);
      this->_state.menu_open_for = index_of_none;
      menu_state_changed = true;
   }
   if (prior) {
      QObject::disconnect(prior, nullptr, this, nullptr);
      prior->removeEventFilter(this); // stop eavesdropping
   }
   this->_root_button.menu = m;
   if (m) {
      //
      // We hook `QMenu::aboutToHide` so we know when the root menu is 
      // dismissed and can update our internal state. However, it's not 
      // safe to hook `QMenu::aboutToShow` because the root menu could 
      // potentially be shown by some other UI, such as a different 
      // DKBreadcrumbBar.
      //
      QObject::connect(m, &QMenu::aboutToHide, this, [this]() {
         if (this->_state.menu_open_for == index_of_root_button) {
            this->_state.menu_open_for = index_of_none;
         }
      });
      //
      // We'll need to "eavesdrop" on this menu just like we eavesdrop 
      // on the menus we create and have sole ownership of.
      //
      m->installEventFilter(this);
   }
   if (!!m != !!prior) {
      this->_re_layout();
   } else if (menu_state_changed) {
      this->repaint();
   }
}

class _ScrollableMenuProxyStyle : public QProxyStyle {
   public:
      virtual int styleHint(
         StyleHint hint,
         const QStyleOption* option = nullptr,
         const QWidget* widget = nullptr,
         QStyleHintReturn* returnData = nullptr
      ) const override {
         if (hint == QStyle::SH_Menu_Scrollable)
            return true;
         return QProxyStyle::styleHint(hint, option, widget, returnData);
      }
};

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
               //
               // Ensure that segment menus are scrollable.
               //
               {
                  auto* proxy = new _ScrollableMenuProxyStyle;
                  proxy->setParent(menu);
                  menu->setStyle(proxy);
               }
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
            assert(seg.menu != nullptr);
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
   //
   // The positioning rules are as follows:
   // 
   //  - A menu button's width includes its horizontal borders.
   // 
   //  - A segment main button generally has no horizontal borders, except:
   // 
   //     - The leading segment has a leading border if there is any space 
   //       or content before it.
   // 
   //     - The trailing segment has a trailing border if it has no menu 
   //       button.
   //
   auto& last_layout = this->_state.last_layout;
   auto& next_layout = this->_state.next_layout;

   bool guaranteed_relayout = force || next_layout.segments_changed;
   next_layout = {};

   {
      auto* textbox = this->_subwidgets.textbox;
      textbox->setGeometry(0, 0, this->width(), this->height());
      //
      // TODO: Make room for current icon, if visible.
      //
   }

   const int  widget_inner_height = this->minimumSizeHint().height() - (this->_styles.border_width * 2);
   const auto menu_button_width   = this->_styles.segment.menu_button_width;

   bool show_root_button = this->_root_button.menu != nullptr;
   {
      int x = this->_styles.border_width;
      int y = this->_styles.border_width;
      this->_root_button.geometry = QRect(x, y, menu_button_width, widget_inner_height);
   };

   auto& segments = this->_segments;

   size_t count = segments.size();
   if (!count) {
      last_layout.count_shown = 0;
      if (guaranteed_relayout)
         this->repaint();
      return;
   }

   //
   // Compute sizes and positions for segments, assuming no constraints on the 
   // available space.
   //
   {
      int x = this->_styles.border_width;
      int y = this->_styles.border_width;
      if (show_root_button)
         x += menu_button_width;

      const auto metrics  = QFontMetrics(this->font());
      const int  margin_h = this->_styles.segment.margins.left() + this->_styles.segment.margins.right();

      for (size_t i = 0; i < count; ++i) {
         auto& seg = segments[i];
         seg.culled = false;

         seg.geometry.main_borders.leading  = false;
         seg.geometry.main_borders.trailing = !seg.menu;

         auto& main = seg.geometry.main_button;
         auto& menu = seg.geometry.menu_button;
         main = metrics.boundingRect(seg.text);
         main.translate(x, -main.y() + y);
         main.setWidth(main.width() + margin_h);
         if (seg.geometry.main_borders.leading) {
            main.setWidth(main.width() + 1);
         }
         if (seg.geometry.main_borders.trailing) {
            main.setWidth(main.width() + 1);
         }
         main.setHeight(widget_inner_height);
         x += main.width();
         if (seg.menu) {
            menu = QRectF(x, y, menu_button_width, widget_inner_height);
            x += menu_button_width;
         } else {
            menu = {};
         }
      }
      this->_segments[0].geometry.main_borders.leading = !show_root_button;
   }
   //
   // Check if the segments all fit in the available space. If not, then count 
   // how many of the trailing segments can fit. If none of them can fit, then 
   // constrain the last segment to the available space and count it as the 
   // only segment that fits.
   //
   size_t count_to_show = count;
   {
      auto& trailing_seg = segments.back();
      int   total_width  = trailing_seg.geometry.main_button.x() + trailing_seg.width();
      int   available    = this->width() - (this->_styles.border_width * 2);
      if (show_root_button) {
         available -= menu_button_width;
      }
      if (available < total_width) {
         //
         // Display only the trailing segment(s). If the root button is not 
         // being shown, then use the space it would be shown in to display 
         // a "..." indicator.
         //
         if (!show_root_button) {
            available -= menu_button_width;
         }
         count_to_show = 0;
         if (available >= trailing_seg.width()) {
            total_width = 0;
            for (count_to_show = 0; count_to_show < count; ++count_to_show) {
               size_t i = count - count_to_show - 1;
               auto   w = this->_segments[i].width();
               if (total_width + w > available)
                  break;
               total_width += w;
            }
         }
         if (count_to_show == 0) {
            //
            // Display only the trailing segment, constraining it to fill the 
            // available space.
            //
            count_to_show = 1;
            trailing_seg.geometry.main_borders.leading = !!this->_root_button.menu;
            auto& main_rect = trailing_seg.geometry.main_button;
            auto& menu_rect = trailing_seg.geometry.menu_button;
            if (trailing_seg.menu) {
               auto main_width = available - menu_button_width;
               main_rect.setWidth(main_width);
               menu_rect.setX(main_rect.x() + main_width);
            } else {
               main_rect.setWidth(available);
            }
         }
      }
   }
   //
   // Update culling state for segments, culling out those that can't fit per the 
   // above checks. Additionally, if the leading segment(s) were culled, then shift 
   // the non-culled segments to take that space.
   //
   if (count == count_to_show) {
      for (auto& seg : segments)
         seg.culled = false;
   } else {
      assert(count_to_show > 0);
      assert(count_to_show < count);
      size_t first_to_show = count - count_to_show;

      int displace_forward  = 0;
      int displace_backward = 0;
      if (!show_root_button) {
         //
         // If the root button isn't already set to show, then use the 
         // space it would occupy to show a "..." indicator. This means 
         // we'll need to displace the segments forward so they don't 
         // cover that space up.
         //
         displace_forward = menu_button_width;
      }
      {
         auto& last_hidden = this->_segments[first_to_show - 1];
         if (last_hidden.menu) {
            displace_backward = last_hidden.geometry.menu_button.right();
         } else {
            displace_backward = last_hidden.geometry.main_button.right();
         }
         if (show_root_button) { // don't cover the root
            displace_backward -= menu_button_width;
         }
      }
      show_root_button = true;
      //
      // Apply displacement and update "culled" flag for all segments.
      //
      int    dx = displace_forward - displace_backward;
      size_t i = 0;
      for (; i < first_to_show; ++i) {
         auto& seg = segments[i];
         seg.culled = true;
         seg.geometry.main_button.translate(dx, 0);
         seg.geometry.menu_button.translate(dx, 0);
      }
      for (; i < count; ++i) {
         auto& seg = segments[i];
         seg.culled = false;
         seg.geometry.main_button.translate(dx, 0);
         seg.geometry.menu_button.translate(dx, 0);
      }
      this->_segments[first_to_show].geometry.main_borders.leading = true;
   }

   last_layout.count_shown = count_to_show;

   if (this->layoutDirection() == Qt::LayoutDirection::RightToLeft) {
      //
      // Mirror all positioning.
      //
      const int widget_width = this->width();
      for (auto& seg : this->_segments) {
         auto& main_rect = seg.geometry.main_button;
         auto& menu_rect = seg.geometry.menu_button;
         const int dst_r = widget_width - main_rect.left();
         int       dst_x = dst_r - main_rect.width();
         if (seg.menu) {
            main_rect.moveLeft(dst_x);
            dst_x -= menu_button_width;
            menu_rect.moveLeft(dst_x);
         } else {
            main_rect.moveLeft(dst_x);
            menu_rect.moveLeft(dst_x);
         }
      }
      this->_root_button.geometry.moveLeft(widget_width - this->_root_button.geometry.left() - menu_button_width);
   }

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
   if (i == index_of_root_button) {
      QMenu* menu = this->_root_button.menu;
      if (menu)
         menu->hide();
      return;
   }
   if (i == index_of_none || i >= this->_segments.size())
      return;
   auto& seg = this->_segments[i];
   if (!seg.menu) {
      if (this->_state.menu_open_for == i) {
         this->_state.menu_open_for = index_of_none; // since there's no QMenu to fire aboutToHide and reset this state
         this->repaint();
      }
      return;
   }
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
         if (this->layoutDirection() == Qt::LayoutDirection::RightToLeft) {
            open_from = this->_root_button.geometry.bottomRight().toPoint();
         } else {
            open_from = this->_root_button.geometry.bottomLeft().toPoint();
         }
         break;
      default:
         {
            const auto& seg = this->_segments[i];
            to_open   = this->_segments[i].menu;
            //
            // We're opening the menu on a parent in order to choose a child to navigate to, and 
            // the parent is not the last segment (i.e. there is a next segment that represents 
            // a child we're already navigated into). We want the menu to be lined up such that 
            // the text of the menu items (representing potential children) aligns with the text 
            // of the child-segment that we're already navigatedinto.
            //
            if (this->layoutDirection() == Qt::LayoutDirection::RightToLeft) {
               if (i + 1 < this->_segments.size()) {
                  auto& next_seg = this->_segments[i + 1];
                  open_from = next_seg.geometry.main_button.bottomLeft().toPoint();
                  //
                  // subtract segment trailing border(?) + menu border(?) + menu inner padding
                  //
                  open_from.rx() -= 18;
               } else {
                  //
                  // Should be impossible, but I'm coding this defensively.
                  //
                  open_from = seg.geometry.menu_button.bottomLeft().toPoint();
               }
            } else {
               open_from = seg.geometry.menu_button.bottomRight().toPoint();
               //
               // subtract menu border(?) + menu inner padding + menu reserved space for icon + space between action icon and label(?)
               //
               open_from.rx() -= 32;
            }
         }
         break;
   }

   //
   // Opening a QMenu causes Qt's core to hijack basically all input events and 
   // route them exclusively to the QMenu (see the notes on "eavesdropping"). 
   // However, it also causes Qt to potentially dispatch a synthetic "mouse leave" 
   // event to whatever widget the mouse was previously over. We need to make sure 
   // that when we receive this "mouse leave" event, we're aware that the mouse has 
   // not actually left.
   //
   {
      bool mouseleave_pending = (this->_state.menu_open_for == index_of_none);
      if (!mouseleave_pending) {
         // last segment has no menu, but we track whether its menu *would* be open
         mouseleave_pending = this->_state.menu_open_for == (this->_segments.size() - 1);
      }

      if (mouseleave_pending) {
         this->_state.next_mouseleave_is_from_menu_opening = true;
      }
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
      // Our own menus use `QMenu::aboutToShow` to update this variable, but that isn't 
      // safe to use for the root menu (since it could be shared with other widgets and 
      // shown for any of them), so for that menu specifically, we update the state here.
      // 
      // (Why use `aboutToShow` at all? The other menus also do setup there.)
      //
      this->_state.menu_open_for = i;
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
         {
            auto& mev = (QMouseEvent&)event;
            //
            // Only eavesdrop on mouse-move events if the mouse is not over the 
            // menu. Otherwise, ensure the menu has priority.
            //
            if (!menu.rect().contains(menu.mapFromGlobal(mev.globalPos()))) {
               this->mouseMoveEvent(&mev);
            }
         }
         break;
      case QEvent::MouseButtonPress:
         {
            auto* mev = (QMouseEvent*)&event;
            if (mev->button() == Qt::LeftButton && this == QApplication::widgetAt(mev->globalPos())) {
               this->_state.last_click_closed_our_menu = true;
            }
         }
         break;
      case QEvent::KeyPress:
         if (!menu.activeAction()) {
            auto kev          = (QKeyEvent*)&event;
            bool was_accepted = kev->isAccepted();
            {
               kev->ignore();
               this->keyPressEvent((QKeyEvent*)&event);
               if (kev->isAccepted())
                  return true;
            }
            if (was_accepted)
               kev->accept();
         }
         break;
   }
   return false;
}
void DKBreadcrumbBar::_on_segment_menu_hidden() {
   if (auto* s = sender()) {
      if (s == this->_root_button.menu) {
         if (this->_state.menu_open_for != index_of_root_button)
            return;
      } else {
         auto closing = s->property("segment-index");
         if (closing.isValid()) {
            if (closing.toInt() != this->_state.menu_open_for)
               return;
         }
      }
   }
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
   const bool   has_root_button = this->_root_button.menu != nullptr;
   const size_t first_segment   = count - this->_state.last_layout.count_shown;
   const size_t first_index     = has_root_button ? index_of_root_button : first_segment;
   const size_t last_index      = count - 1;

   size_t i = this->_state.hovered_segment;
   if (i == index_of_none) {
      if (left) {
         i = last_index;
      } else {
         i = first_index;
      }
   } else if (i == index_of_root_button) {
      if (left)
         i = last_index;
      else
         i = first_segment;
   } else {
      if (left) {
         if (i == first_index)
            i = last_index;
         else if (i == first_segment && has_root_button)
            i = index_of_root_button;
         else
            --i;
      } else {
         if (i == index_of_root_button)
            i = first_segment;
         else if (i == last_index)
            i = first_index;
         else
            ++i;
      }
   }
   this->_on_segment_hovered(i);
}
void DKBreadcrumbBar::_on_vertical_arrow_key(bool up) {
   {
      //
      // If a menu is already open, but none of its actions have focus, then 
      // move focus to one of its actions.
      //
      size_t i = this->_state.menu_open_for;
      if (i != index_of_none) {
         QMenu* menu = nullptr;
         if (i == index_of_root_button)
            menu = this->_root_button.menu;
         else if (i < this->_segments.size())
            menu = this->_segments[i].menu;
         if (menu && menu->isVisible()) {
            if (!menu->activeAction()) {
               auto actions = menu->actions();
               if (!actions.isEmpty()) {
                  size_t i = up ? actions.size() - 1 : 0;
                  menu->setActiveAction(actions[i]);
               }
            }
            return;
         }
      }
   }
   //
   // Otherwise, try to open a menu as appropriate.
   //
   auto _hover_root_or_first = [this]() {
      size_t first = 0;
      if (this->_root_button.menu)
         first = index_of_root_button;
      this->_on_segment_hovered(first);
      this->repaint();
   };
   //
   auto hover_idx = this->_state.hovered_segment;
   if (hover_idx == index_of_none) {
      //
      // If there is no "hovered" segment (i.e. because we're hovering 
      // the empty gutter after all segments), then hover the root 
      // button.
      //
      _hover_root_or_first();
      return;
   }
   if (hover_idx == index_of_root_button) {
      this->_open_menu(hover_idx);
      return;
   }
   if (hover_idx >= this->_segments.size())
      return;
   if (hover_idx == this->_segments.size() - 1) {
      //
      // Vertical arrow keys on the trailing (menuless) segment also 
      // shift hover to the root button.
      //
      _hover_root_or_first();
      return;
   }
   this->_open_menu(hover_idx);
}

void DKBreadcrumbBar::_begin_text_editing() {
   auto* textbox = this->_subwidgets.textbox;
   {
      const auto blocker = QSignalBlocker(textbox);
      textbox->setText(this->path());
   }
   textbox->setVisible(true);
   textbox->setFocus();
   textbox->selectAll();
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

void DKBreadcrumbBar::_recache_icons() {
   qreal dx = this->layoutDirection() == Qt::LayoutDirection::LeftToRight ? 1 : -1;
   {
      auto& path = this->_state.icons.chevron_base;
      path.clear();
      path.moveTo(QPointF{ -dx, -2 });
      path.lineTo(QPointF{  dx,  0 });
      path.lineTo(QPointF{ -dx,  2 });
   }
   {
      auto& path = this->_state.icons.chevron_open;
      path.clear();
      path.moveTo(QPointF{ -2, -1 });
      path.lineTo(QPointF{  0,  1 });
      path.lineTo(QPointF{  2, -1 });
   }
   {
      auto& path = this->_state.icons.chevron_more;
      path.clear();
      path.moveTo(QPointF{ -dx*2.5, -2 });
      path.lineTo(QPointF{    -0.5,  0 });
      path.lineTo(QPointF{ -dx*2.5,  2 });
      path.moveTo(QPointF{     0.5, -2 });
      path.lineTo(QPointF{  dx*2.5,  0 });
      path.lineTo(QPointF{     0.5,  2 });
   }
   this->_state.icons.cached = true;
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
   /*virtual*/ void DKBreadcrumbBar::focusOutEvent(QFocusEvent* event) {
      const auto* focused = QApplication::focusWidget();
      auto*       textbox = this->_subwidgets.textbox;
      if (focused) {
         if (focused == this) // spurious activation
            return;
         if (focused == textbox)
            return;
         for (auto& seg : this->_segments)
            if (focused == seg.menu)
               return;
      }
      //
      // Focus has not been transferred to a sub-widget or submenu.
      //
      this->_on_segment_hovered(index_of_none);
      if (textbox->isVisible())
         textbox->setVisible(false);
   }
   /*virtual*/ void DKBreadcrumbBar::keyPressEvent(QKeyEvent* event) {
      //
      // QKeyEvents are accepted by default, but we explicitly accept them 
      // in order to help the menu-eavesdropping hack.
      //
      switch (event->key()) {
         case Qt::Key::Key_Up:
         case Qt::Key::Key_Down:
            event->accept();
            this->_on_vertical_arrow_key(event->key() == Qt::Key::Key_Up);
            break;
         case Qt::Key::Key_Right:
         case Qt::Key::Key_Left:
            event->accept();
            {
               bool left = event->key() == Qt::Key::Key_Left;
               if (this->layoutDirection() == Qt::LayoutDirection::RightToLeft)
                  left = !left;
               this->_on_horizontal_arrow_key(left);
            }
            break;
         case Qt::Key::Key_Enter:
         case Qt::Key::Key_Space:
            switch (size_t i = this->_state.hovered_segment) {
               case index_of_none:
                  break;
               case index_of_root_button:
                  if (this->textEditingAllowed()) {
                     event->accept();
                     this->_begin_text_editing();
                  }
                  break;
               default:
                  event->accept();
                  if (i < this->_segments.size())
                     this->_on_segment_clicked(this->_segments[i]);
                  break;
            }
            break;
         case Qt::Key::Key_Escape:
            //
            // Special-case: The trailing segment never has a menu, but we track 
            // whether its menu *would* be open as a result of keyboard navigation 
            // so that navigating to and past that segment properly pops the menu 
            // of the adjacent segment. In essence, the widget works less in terms 
            // of "a menu being open" and more in terms of the widget "being in 
            // 'menu mode.'"
            // 
            // If we're "in menu mode" but a menu isn't open, then Esc should take 
            // us out of "menu mode" consistent with the Windows 10 native widget.
            //
            if (this->_state.menu_open_for == this->_segments.size() - 1) {
               event->accept();
               this->_close_menu(this->_state.menu_open_for);
            }
            break;
      }
   }
   /*virtual*/ void DKBreadcrumbBar::leaveEvent(QEvent* event) {
      if (this->_state.next_mouseleave_is_from_menu_opening) {
         this->_state.next_mouseleave_is_from_menu_opening = false;
         return;
      }
      this->_on_segment_hovered(index_of_none);
   }
   /*virtual*/ void DKBreadcrumbBar::mouseMoveEvent(QMouseEvent* event) {
      //auto pos = event->localPos();
      auto pos = this->mapFromGlobal(event->globalPos());
      // The event may have been originally delivered to a QMenu before we took it, 
      // so we can't trust its own localPos(). Refer to the "menu eavesdropping" 
      // code and its comments for further information.

      if (this->_root_button.menu) {
         if (this->_root_button.geometry.contains(pos)) {
            this->_on_segment_hovered(index_of_root_button);
            return;
         }
      }
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
         if (this->_state.last_click_closed_our_menu) {
            //
            // We're responding to the same click that closed our menu. We will have 
            // already received `QMenu::aboutToHide` and cleared our "menu open for" 
            // state value, so we have no other way of knowing this. If we don't exit 
            // here, then we'll re-open the menu that was just closed.
            //
            this->_state.last_click_closed_our_menu = false;
            return;
         }

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
               this->_open_menu(i);
               return;
            }
         }
         if (this->textEditingAllowed()) {
            event->accept();
            this->_begin_text_editing();
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
      const auto _segment_state = [this, is_disabled, is_menu_open](size_t i) {
         if (is_disabled)
            return segment_paint_state::disabled;
         if (!is_menu_open && i == this->_state.hovered_segment)
            return segment_paint_state::hovered;
         return segment_paint_state::normal;
      };

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

      if (!this->_state.icons.cached) {
         this->_recache_icons();
      }

      const auto& _colors_for_segment = [this](segment_paint_state state) -> SegmentPalette& {
         switch (state) {
            case segment_paint_state::disabled: return this->_styles.segment.colors.disabled;
            case segment_paint_state::hovered:  return this->_styles.segment.colors.hovered;
         }
         return this->_styles.segment.colors.normal;
      };

      if (this->_root_button.menu) {
         const auto& colors = _colors_for_segment(_segment_state(index_of_root_button));
         painter.setBrush(colors.menu_button.fill);
         painter.setPen(colors.menu_button.line);
         painter.drawRect(this->_root_button.geometry - qt_border_jank);
         //
         // Draw icon.
         //
         painter.save();
         painter.setPen(colors.menu_button.icon);
         painter.setRenderHint(QPainter::RenderHint::Antialiasing);
         painter.translate(this->_root_button.geometry.center());
         const auto& icon =
            (this->_state.last_layout.count_shown < this->_segments.size()) ?
               this->_state.icons.chevron_more
            :
               (this->_state.menu_open_for == index_of_root_button) ?
                  this->_state.icons.chevron_open
               :
                  this->_state.icons.chevron_base
         ;
         painter.drawPath(icon);
         painter.restore();
      } else if (this->_state.last_layout.count_shown < this->_segments.size()) {
         //
         // Draw a "..." button to indicate that not all segments are shown.
         // 
         // QPainterPath doesn't have `drawPoint`, and if QPainter is told to 
         // draw a line from a point to the same point, it produces no output 
         // rather than a single point; so we can't prepare a path for this in 
         // advance.
         //
         painter.save();
         painter.setPen(QPen(this->palette().color(QPalette::Text), 1.5F));
         painter.setRenderHint(QPainter::RenderHint::Antialiasing);
         painter.translate(this->_root_button.geometry.center());
         painter.drawPoint(QPointF{ -3, 0 });
         painter.drawPoint(QPointF{  0, 0 });
         painter.drawPoint(QPointF{  3, 0 });
         painter.restore();
      }

      painter.setFont(this->font());
      for (size_t i = 0; i < this->_segments.size(); ++i) {
         const auto& segment = this->_segments[i];
         if (segment.culled) {
            continue;
         }
         segment.repaint(*this, painter, _segment_state(i), i);
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
         //
         // It is not enough to blindly eavesdrop on any menu that trips this 
         // event filter, because breadcrumb bars can be given a root menu that 
         // may be shared with other widgets, and the bars have to install an 
         // event filter on that root menu. If the root menu is opened by some 
         // other widget, then we'll receive the events that the menu intercepts 
         // from that other widget.
         // 
         // We need to make sure that if the menu we're getting events for is 
         // our root menu, *we* know that the root menu is open i.e. the root 
         // menu was opened *via us.*
         //
         bool eavesdrop = false;
         switch (size_t i = this->_state.menu_open_for) {
            case index_of_none:
               break;
            case index_of_root_button:
               eavesdrop = casted == this->_root_button.menu;
               break;
            default:
               if (i < this->_segments.size()) {
                  eavesdrop = casted == this->_segments[i].menu;
               }
               break;
         }
         if (eavesdrop && this->_do_menu_eavesdropping(*casted, *event))
            return true;
      }
      return QWidget::eventFilter(watched, event);
   }
#pragma endregion