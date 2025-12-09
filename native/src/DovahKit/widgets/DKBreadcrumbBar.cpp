#include "./DKBreadcrumbBar.h"
#include <algorithm> // std::reverse
#include <QApplication>
#include <QCommonStyle>
#include <QMetaMethod>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QProxyStyle>
#include <QStyle>
#include <QStyleOption>

namespace {
   // When drawing a rect with an odd pen (stroke) width, Qt rounds 
   // the stroke down. This means that for a rect with a 1px border, 
   // the left and top borders are inside the rect, and the right and 
   // bottom borders are outside the rect. As you might expect, this 
   // is very silly and we have to adjust for it basically everywhere.
   static constexpr const QMargins qt_border_jank = { 0, 0, 1, 1 };

   //
   // Qt's border jank is hard to properly account for and adjust for 
   // throughout the codebase, so some of our calculations end up being 
   // completely correct for hit testing but off for rendering, or off 
   // for both, and it's just a massive headache to try and figure out 
   // why. Easier to just apply spot corrections during rendering. 
   // This constant exists just as a succinct means to clearly mark 
   // those corrections.
   //
   constexpr const bool janky_corrections = true;
}

// Forces a QMenu to always use scrolling rather than expanding to multiple columns.
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

#pragma region DKBreadcrumbBar::Styles
   DKBreadcrumbBar::Styles::Styles() {
      this->border_width = 1;

      this->segment.margins = { 5, 3, 5, 3 };
      this->segment.menu_button_width = 15;

      this->segment.colors.normal = {
         .main_button = {
            .fill = QColor(255, 255, 255),
            .line = QPen(QColor(224, 224, 224), 0),
            .text = QPen(QColor(0, 0, 0), 0),
         },
         .menu_button = {
            .fill = QColor(255, 255, 255),
            .line = QPen(QColor(224, 224, 224), 0),
            .icon = QPen(QColor(128, 128, 128), 1.5, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin),
         },
      };
      this->segment.colors.hovered = {
         .main_button = {
            .fill = QColor(229, 243, 255),
            .line = QPen(QColor(204, 232, 255), 0),
            .text = this->segment.colors.normal.main_button.text,
         },
         .menu_button = {
            .fill = QColor(229, 243, 255),
            .line = QPen(QColor(204, 232, 255), 0),
            .icon = this->segment.colors.normal.menu_button.icon,
         },
      };
      this->segment.colors.disabled = {
         .main_button = {
            .fill = QColor(0, 0, 0, 0),
            .line = QColor(0, 0, 0, 0),
            .text = this->segment.colors.normal.main_button.text,
         },
         .menu_button = {
            .fill = QColor(0, 0, 0, 0),
            .line = QColor(0, 0, 0, 0),
            .icon = QPen(this->segment.colors.normal.main_button.text.color(), 1.5, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin),
         },
      };
   }
#pragma endregion

#pragma region DKBreadcrumbBar::segment
   unsigned int DKBreadcrumbBar::segment::width() const noexcept {
      int width = this->geometry.main_button.width();
      if (this->has_menu)
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

      if (this->has_menu) {
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
      QObject::connect(textbox, &QLineEdit::editingFinished, this, &DKBreadcrumbBar::finishTextEditing);
   }

   this->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
   this->setMouseTracking(true);

   {
      auto& menu = this->_subwidgets.segment_menu;
      this->_start_menu_eavesdropping(menu);
      QObject::connect(&menu, &QMenu::aboutToHide, this, &DKBreadcrumbBar::_on_segment_menu_hidden);
      QObject::connect(&menu, &QMenu::triggered,   this, &DKBreadcrumbBar::_on_segment_menu_item_selected);
      {
         auto* proxy = new _ScrollableMenuProxyStyle;
         proxy->setParent(&menu);
         menu.setStyle(proxy);
      }
   }
}

QAbstractItemModel* DKBreadcrumbBar::model() const noexcept {
   return this->_data.model;
}
void DKBreadcrumbBar::setModel(QAbstractItemModel* model) {
   auto* prior = this->model();
   if (prior == model)
      return;
   this->cancelTextEditing();
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
      QObject::connect(model, &QAbstractItemModel::rowsMoved,   this, &DKBreadcrumbBar::_on_items_moved);
      QObject::connect(model, &QAbstractItemModel::dataChanged, this, &DKBreadcrumbBar::_on_data_changed);
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
   if (this->isEditingText()) {
      this->_update_textbox_value();
      this->_subwidgets.textbox->selectAll();
   }
}

void DKBreadcrumbBar::setTextEditingAllowed(bool v) {
   if (v == this->_text_editing.allowed)
      return;
   this->_text_editing.allowed = v;
   if (!v)
      this->cancelTextEditing();
}

void DKBreadcrumbBar::setTextSeparator(QChar c) {
   auto& prop = this->_text_editing.separator;
   if (prop == c)
      return;
   prop = c;
   //
   if (this->textEditingAllowed() && this->isEditingText()) {
      this->_update_textbox_value();
      this->_subwidgets.textbox->selectAll();
   }
}

void DKBreadcrumbBar::setCaseSensitivity(Qt::CaseSensitivity v) {
   this->_text_editing.case_sensitivity = v;
}

bool DKBreadcrumbBar::isEditingText() const noexcept {
   return this->_subwidgets.textbox->isVisible();
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
bool DKBreadcrumbBar::setPath(QString path) {
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

QMenu* DKBreadcrumbBar::rootMenu() const noexcept {
   return this->_root_button.menu;
}
void DKBreadcrumbBar::setRootMenu(QMenu* m) {
   assert(m != &this->_subwidgets.segment_menu);
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
      QObject::connect(m, &QMenu::aboutToHide, this, &DKBreadcrumbBar::_on_root_menu_hidden);
      //
      // We'll need to "eavesdrop" on this menu just like we eavesdrop 
      // on the segment menu.
      //
      this->_start_menu_eavesdropping(*m);
   }
   if (!!m != !!prior) {
      this->_re_layout();
   } else if (menu_state_changed) {
      this->repaint();
   }
}

void DKBreadcrumbBar::setStyles(const Styles& s) {
   this->_styles = s;
   this->update();
}

void DKBreadcrumbBar::_on_navigated() {
   //
   // Teardown.
   //
   this->setFocusProxy(nullptr);
   this->_segments.clear();
   this->_state.hovered_segment = index_of_none;
   this->_state.menu_open_for   = index_of_none;
   {
      auto&      menu    = this->_subwidgets.segment_menu;
      const auto blocker = QSignalBlocker(&menu); // we don't want to be notified of menu closure here
      menu.hide();
   }
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
               seg.has_menu = true;
            }
         }
      } while (qmi = model->parent(qmi), basis = false, qmi.isValid());
      std::reverse(list.begin(), list.end());
   }
   this->_state.hovered_segment = index_of_none;
   this->_state.next_layout.segments_changed = true;
   this->_re_layout();

   //
   // Emit signals. Don't bother stringifying to a path for signals' sake if nothing 
   // is listening for path-related signals.
   //
   if (!this->signalsBlocked()) {
      if (this->isSignalConnected(QMetaMethod::fromSignal(&DKBreadcrumbBar::currentPathChanged))) {
         auto qmi  = this->currentIndex();
         auto path = this->path();
         emit this->currentIndexChanged(qmi);
         emit this->currentPathChanged(path);
      } else {
         emit this->currentIndexChanged(this->currentIndex());
      }
   }
}
void DKBreadcrumbBar::_on_data_changed(const QModelIndex& qmi) {
   auto&   list  = this->_segments;
   bool    found = false;
   QString label;
   QIcon   icon;
   for (size_t i = 0; i < list.size(); ++i) {
      auto& seg = list[i];
      if (seg.qmi == qmi) {
         label = this->model()->data(qmi, Qt::DisplayRole).toString();
         icon  = this->model()->data(qmi, Qt::DecorationRole).value<QIcon>();
         seg.text = label;
         found = true;
         break;
      }
   }
   if (found) {
      this->_state.next_layout.segments_changed = true;
      this->_re_layout();
   }
   if (auto* action = this->_segment_menu_action_by_qmi(qmi)) {
      if (!found) {
         label = this->model()->data(qmi, Qt::DisplayRole).toString();
         icon  = this->model()->data(qmi, Qt::DecorationRole).value<QIcon>();
      }
      action->setText(label);
      action->setIcon(icon);
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
   if (auto* action = this->_segment_menu_action_by_qmi(qmi)) {
      this->_subwidgets.segment_menu.removeAction(action);
   }
   if (!found) {
      return false;
   }
   this->setCurrentIndex(parent);
   return true;
}
void DKBreadcrumbBar::_on_items_moved(const QModelIndex& src_parent, int first, int last, const QModelIndex& dst_parent) {
   if (auto size = this->_segments.size(); size > 1) {
      bool hierarchy_changed = false;
      for (size_t i = 0; i < size - 1; ++i) {
         const auto& seg = this->_segments[i];
         if (seg.qmi == src_parent) {
            hierarchy_changed = true;
            break;
         }
      }
      if (hierarchy_changed) {
         const auto blocker = QSignalBlocker(this); // don't emit index-/path-changed signals
         this->_on_navigated(); // rebuild the hierarchy
      }
   }

   auto mof = this->_state.menu_open_for;
   switch (mof) {
      case index_of_none:
      case index_of_root_button:
         return;
   }
   if (mof < this->_segments.size()) {
      const auto& seg = this->_segments[mof];
      if (seg.qmi == src_parent || seg.qmi == dst_parent) {
         auto& menu = this->_subwidgets.segment_menu;
         bool  has_active_action = menu.activeAction();
         menu.setUpdatesEnabled(false);
         this->_build_segment_menu(seg.qmi);
         menu.setUpdatesEnabled(true);
         if (has_active_action && !menu.activeAction()) {
            auto actions = menu.actions();
            if (!actions.isEmpty())
               menu.setActiveAction(actions[0]);
         }
      }
   }
}
void DKBreadcrumbBar::_build_segment_menu(const QModelIndex& qmi) {
   auto& menu = this->_subwidgets.segment_menu;
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
      action->setProperty("qpmi", QPersistentModelIndex(child_qmi));
      if (icon.canConvert<QIcon>()) {
         action->setIcon(icon.value<QIcon>());
      }
      if (selected_child.isValid() && child_qmi == selected_child) {
         auto font = action->font();
         font.setBold(true);
         action->setFont(font);
      }
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
         seg.geometry.main_borders.trailing = !seg.has_menu;

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
         if (seg.has_menu) {
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
            if (trailing_seg.has_menu) {
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
         if (last_hidden.has_menu) {
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
         if (seg.has_menu) {
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

/*static*/ int DKBreadcrumbBar::_guesstimate_menu_text_x_offset(const QMenu& menu) {
   //
   // Qt offers no convenient getters for the text position/offset within a menu, 
   // even via QStyle. Most of their offsets are hardcoded constants which were 
   // only occasionally given actual names. The code that determines text position 
   // is scattered across QMenu and QWindowsStyle/QCommonStyle, composed from a 
   // handful of different calculations run in near-total isolation from one 
   // another.
   // 
   // This function is going to be ugly.
   //
   menu.ensurePolished();
   const QStyle* style = menu.style();
   if (const auto* proxy = qobject_cast<const QProxyStyle*>(style))
      style = proxy->baseStyle();

   QStyleOption opt;
   opt.init(&menu);

   int left = 0;
   //
   // Apply the effects of QMenuPrivate::updateActionRects.
   //
   left += style->pixelMetric(QStyle::PM_MenuPanelWidth, &opt, &menu);
   left += style->pixelMetric(QStyle::PM_MenuHMargin, &opt, &menu);
   //
   int icon_width = 0;
   for (auto* action : menu.actions()) {
      auto icon = action->icon();
      if (!icon.isNull()) {
         icon_width = style->pixelMetric(QStyle::PM_SmallIconSize, &opt, &menu) + 4;
         break;
      }
   }
   //
   // Apply the effects of the styles.
   //
   if (!style) {
      return left;
   }
   //
   // As of Qt 5, QWindowsStyle has no public header that we can include 
   // for direct casts (and I'm not sure that would build on non-Windows 
   // targets anyway).
   //
   if (style->inherits("QWindowsStyle")) {
      // Based on QWindowsStyle::sizeFromContents(CT_MenuItem, ...).
      
      // Normally, this subtraction is only made if the icon width is 
      // zero, but testing in Windows 10 suggests it's always needed.
      left -= 6;

      // Normally, this would be the maximum of the max icon width or 
      // of the windowsCheckMarkWidth constant. Testing in Windows 10 
      // suggests it's always needed.
      left += 12; // QWindowsStylePrivate::windowsCheckMarkWidth

      // This shouldn't have even been here. This was a mistake. But 
      // testing on Windows 10 suggests it's always needed.
      left += 12;
   } else if (qobject_cast<const QCommonStyle*>(style)) {
      // Based on QCommonStyle::sizeFromContents(CT_MenuItem, ...).

      if (icon_width > 0) {
         left += icon_width;
         left += 6;
      }
      if (icon_width > 2) {
         left += 2;
      }
   }
   return left;
}
QPoint DKBreadcrumbBar::_compute_menu_position(size_t segment_index) const {
   switch (segment_index) {
      case index_of_none:
         return {};
      case index_of_root_button:
         assert(!!this->_root_button.menu);
         if (this->layoutDirection() == Qt::LayoutDirection::RightToLeft) {
            auto pos = this->_root_button.geometry.bottomRight().toPoint();
            pos.rx() -= this->_root_button.menu->sizeHint().width();
            return pos;
         }
         return this->_root_button.geometry.bottomLeft().toPoint();
   }

   if (segment_index >= this->_segments.size())
      return {};
   auto& seg = this->_segments[segment_index];
   if (!seg.has_menu)
      return {};

   QPoint point;
   //
   // We're opening the menu on a parent in order to choose a child to navigate to, and 
   // the parent is not the last segment (i.e. there is a next segment that represents 
   // a child we're already navigated into). We want the menu to be lined up such that 
   // the text of the menu items (representing potential children) aligns with the text 
   // of the child-segment that we're already navigated into.
   //
   if (this->layoutDirection() == Qt::LayoutDirection::RightToLeft) {
      if (segment_index + 1 >= this->_segments.size()) // should be impossible, since the trailing segment isn't allowed to have a menu
         return seg.geometry.menu_button.bottomLeft().toPoint();
      point = this->_segments[segment_index + 1].geometry.main_button.bottomLeft().toPoint();
      point.rx() -= this->_styles.segment.margins.left();
      if constexpr (janky_corrections) {
         //
         // one of the three is true for RTL:
         // 
         //  - we need to subtract the margin twice
         //  - we need to subtract the margin once, and subtract 5px
         //  - we need to subtract 10px
         // 
         // not sure which. there's some kind of offset being applied to the menu position 
         // that we're compensating for. Qt's code for that is a rat's nest and i don't 
         // have the energy to scurry around it any longer
         //
         point.rx() -= this->_styles.segment.margins.left();
      }
   } else {
      point = seg.geometry.menu_button.bottomLeft().toPoint();
      point.rx() += this->_styles.segment.margins.left();
   }
   point.rx() -= _guesstimate_menu_text_x_offset(this->_subwidgets.segment_menu);
   return point;
}
void DKBreadcrumbBar::_open_menu(size_t i) {
   QMenu* to_open = nullptr;
   switch (i) {
      case index_of_none:
         break;
      case index_of_root_button:
         to_open = this->_root_button.menu;
         if (!to_open)
            return;
         break;
      default:
         if (this->_segments[i].has_menu)
            to_open = &this->_subwidgets.segment_menu;
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
   if (to_open) {
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
   this->_state.menu_open_for = index_of_none; // so that the menu-close handlers don't do anything
   {
      //
      // Only hide the previous menu if we're switching between two different 
      // QMenu instances, or if we're switching to no QMenu instance.
      //
      bool root_prior = this->_state.menu_open_for == index_of_root_button;
      bool root_after = i == index_of_root_button;

      bool hide_previous_menu = root_prior != root_after;
      if (!hide_previous_menu && i != index_of_none && !to_open)
         hide_previous_menu = true;

      if (hide_previous_menu) {
         if (root_prior) {
            if (QMenu* menu = this->_root_button.menu)
               menu->hide();
         } else {
            this->_subwidgets.segment_menu.hide();
         }
      }
   }
   this->_state.menu_open_for = i;
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
      return;
   }
   if (i < this->_segments.size()) {
      this->_subwidgets.segment_menu.setUpdatesEnabled(false);
      this->_build_segment_menu(this->_segments[i].qmi);
      this->_subwidgets.segment_menu.setUpdatesEnabled(true);
   }
   to_open->popup(this->mapToGlobal(this->_compute_menu_position(i)));
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
   // to still receive those input events ourselves (e.g. as a QMenuBar would), 
   // so we need to install ourselves as an event filter on the menu.
   //
   menu.installEventFilter(this);
}
bool DKBreadcrumbBar::_do_menu_eavesdropping(QMenu& menu, QEvent& event) {
   if (QApplication::activePopupWidget() != &menu)
      return false;
   //
   // Peek at application-wide events that are being re-routed to our 
   // open menu, in case those events *would've* gone to us and we 
   // still want to handle them.
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
void DKBreadcrumbBar::_on_root_menu_hidden() {
   switch (this->_state.menu_open_for) {
      case index_of_root_button:
         break;
      case index_of_none:
      default:
         return;
   }
   this->_state.menu_open_for = index_of_none;
   this->repaint();
}
void DKBreadcrumbBar::_on_segment_menu_item_selected(QAction* action) {
   if (!action)
      return;
   auto data = action->property("qpmi");
   if (!data.isValid() || !data.canConvert<QPersistentModelIndex>())
      return;
   this->setCurrentIndex(data.value<QPersistentModelIndex>());
}
void DKBreadcrumbBar::_on_segment_menu_hidden() {
   switch (this->_state.menu_open_for) {
      case index_of_root_button:
      case index_of_none:
         return;
   }
   this->_state.menu_open_for = index_of_none;
   this->repaint();
}
void DKBreadcrumbBar::_close_any_open_menu() {
   this->_state.menu_open_for = index_of_none;
   if (QMenu* menu = this->_root_button.menu)
      menu->hide();
   this->_subwidgets.segment_menu.hide();
   this->repaint();
}

QAction* DKBreadcrumbBar::_segment_menu_action_by_qmi(const QModelIndex& qmi) {
   const auto mof = this->_state.menu_open_for;
   switch (mof) {
      case index_of_none:
      case index_of_root_button:
         return nullptr;
   }
   if (mof >= this->_segments.size())
      return nullptr;
   if (qmi.parent() != this->_segments[mof].qmi)
      return nullptr;

   auto& menu = this->_subwidgets.segment_menu;
   for (auto* action : menu.actions()) {
      auto action_qmi = action->property("qpmi").value<QPersistentModelIndex>();
      if (action_qmi == qmi)
         return action;
   }
   return nullptr;
}

void DKBreadcrumbBar::_on_segment_clicked(const segment& seg) {
   this->_close_any_open_menu();
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
            menu = &this->_subwidgets.segment_menu;
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
            this->cancelTextEditing();
            //
            this->_state.menu_open_for = index_of_none;
            this->_subwidgets.segment_menu.hide();
            if (QMenu* menu = this->_root_button.menu)
               menu->hide();
            //
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
         if (focused == &this->_subwidgets.segment_menu)
            return;
      }
      //
      // Focus has not been transferred to a sub-widget or submenu.
      //
      this->_on_segment_hovered(index_of_none);
      this->cancelTextEditing();
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
               this->_close_any_open_menu();
            }
            break;
      }
   }
   /*virtual*/ void DKBreadcrumbBar::leaveEvent(QEvent* event) {
      if (this->_state.next_mouseleave_is_from_menu_opening) {
         this->_state.next_mouseleave_is_from_menu_opening = false;
         return;
      }
      if (this->_state.menu_open_for != index_of_none) {
         //
         // This is not strictly accurate. The native Windows breadcrumb bar sets 
         // the hovered segment to "none" but doesn't change which menu is open, 
         // so the current menu stays open but arrow keys navigate as if you're 
         // not hovering over any segment. But that's a bit confusing if anything.
         //
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

      if (this->isEditingText())
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
      {
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
               eavesdrop = watched == this->_root_button.menu;
               break;
            default:
               eavesdrop = watched == &this->_subwidgets.segment_menu;
               break;
         }
         if (eavesdrop && this->_do_menu_eavesdropping(*qobject_cast<QMenu*>(watched), *event))
            return true;
      }
      return QWidget::eventFilter(watched, event);
   }
#pragma endregion

#pragma region Slots
   void DKBreadcrumbBar::beginTextEditing() {
      if (this->isEditingText() || !this->textEditingAllowed())
         return;
      this->_begin_text_editing();
   }
   void DKBreadcrumbBar::cancelTextEditing() {
      if (!this->isEditingText())
         return;
      auto* textbox = this->_subwidgets.textbox;
      bool  focus   = textbox->hasFocus();
      textbox->setVisible(false);
      if (focus)
         this->setFocus();
      this->repaint();
   }
   void DKBreadcrumbBar::finishTextEditing() {
      if (!this->isEditingText())
         return;
      auto* textbox = this->_subwidgets.textbox;
      textbox->setVisible(false);
      this->setPath(textbox->text());
      this->setFocus();
      this->repaint();
   }
#pragma endregion