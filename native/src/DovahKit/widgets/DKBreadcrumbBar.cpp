#include "./DKBreadcrumbBar.h"
#include <algorithm> // std::reverse
#include <QCommonStyle>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionButton>

DKBreadcrumbBar::DKBreadcrumbBar(QWidget* parent) : QFrame(parent) {
   this->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
   this->setMinimumWidth(100);
   //
   this->setFrameStyle(QFrame::Shape::Box | QFrame::Shadow::Sunken);
   {
      QStyleOptionButton opt;
      opt.features |= QStyleOptionButton::ButtonFeature::Flat | QStyleOptionButton::ButtonFeature::HasMenu;

      auto* style = this->style();
      QSize size  = style->sizeFromContents(
         QStyle::CT_PushButton,
         &opt,
         QSize{},
         nullptr
      );
      const auto margins = this->contentsMargins();
      this->setFixedHeight(size.height() + margins.top() + margins.bottom());
   }

   {
      auto* textbox = this->_subwidgets.textbox = new QLineEdit(this);
      textbox->setVisible(false);
   }

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
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
      auto& list = this->_subwidgets.segments;
      for (auto& seg : list) {
         seg.button->setMenu(nullptr);
         seg.button->deleteLater();
         seg.button = nullptr;
         if (seg.menu) {
            seg.menu->setVisible(false);
            seg.menu->deleteLater();
            seg.menu = nullptr;
         }
      }
      list.clear();
   }
   //
   // Rebuild.
   //
   {
      auto* model = this->model();
      auto& list  = this->_subwidgets.segments;
      auto  qmi   = this->currentIndex();
      bool  basis = true;
      do {
         auto    flags = model->flags(qmi);
         QString label = model->data(qmi, Qt::DisplayRole).toString();
         if (label.isEmpty() && !qmi.isValid()) {
            break;
         }
         auto& seg = this->_subwidgets.segments.emplace_back();
         seg.qmi = qmi;
         auto* button = seg.button = new QPushButton(this);
         button->setFlat(true);
         button->setText(label);
         button->setToolTip(label);
         if (!basis) { // don't show a menu on the current-index item
            if (!(flags & Qt::ItemNeverHasChildren) && model->rowCount(qmi) > 0) {
               auto* menu = seg.menu = new QMenu(button);
               button->setMenu(menu);
               QObject::connect(menu, &QMenu::aboutToShow, this, [this, menu, qmi]() {
                  this->_set_up_menu(*menu, qmi);
               });
            }
         }

         QPersistentModelIndex qpmi = seg.qmi;
         QObject::connect(button, &QPushButton::clicked, this, [this, qpmi]() {
            this->setCurrentIndex(qpmi);
         });
      } while (qmi = model->parent(qmi), basis = false, qmi.isValid());
      std::reverse(list.begin(), list.end());
      if (!list.empty()) {
         this->setFocusProxy(list[0].button);
         for (size_t i = 0; i < list.size() - 1; ++i) {
            auto* a = list[i].button;
            auto* b = list[i + 1].button;
            QWidget::setTabOrder(a, b);
         }
      }
   }
   this->_state.next_layout.segments_changed = true;
   this->_re_layout();

   emit this->currentIndexChanged(this->currentIndex());
}
void DKBreadcrumbBar::_on_data_changed(const QModelIndex& qmi) {
   auto& list  = this->_subwidgets.segments;
   bool  found = false;
   for (size_t i = 0; i < list.size(); ++i) {
      auto& seg = list[i];
      if (seg.qmi == qmi) {
         QString label = this->model()->data(seg.qmi, Qt::DisplayRole).toString();
         seg.button->setText(label);
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
   
   auto& list  = this->_subwidgets.segments;
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
      auto& list = this->_subwidgets.segments;
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

   auto& segments = this->_subwidgets.segments;

   auto*  style = this->style();
   size_t count = segments.size();
   if (!count) {
      last_layout.any_truncated = false;
      last_layout.count_shown   = 0;
      return;
   }

   int height      = segments[0].button->sizeHint().height();
   int total_width = 0;
   std::vector<int> widths;
   widths.resize(count);
   for (size_t i = 0; i < count; ++i) {
      auto* button = segments[i].button;
      //
      // Qt buttons are programmed to ensure a comfortable amount of padding 
      // around their text, as long as they aren't being crammed into a narrow 
      // space. There's no way to disable this padding, nor a way to query what 
      // size the button would have without it. It's a minimum size hardcoded 
      // into the stock QStyle subclasses.
      // 
      if (auto* casted = qobject_cast<QCommonStyle*>(button->style())) {
         //
         // QCommonStyle computes the size before minimums are applied, so we 
         // can try to forcibly invoke it and not any override.
         //
         QStyleOptionButton opt;
         opt.text = button->text();
         opt.features |= QStyleOptionButton::ButtonFeature::Flat;
         if (i + 1 < count)
            opt.features |= QStyleOptionButton::ButtonFeature::HasMenu;
         QSize contents_size;
         {
            contents_size = QFontMetrics(button->font()).size(Qt::TextShowMnemonic, opt.text);
            if (button->menu())
               contents_size.rwidth() += casted->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, button);
         }
         QSize button_size = casted->QCommonStyle::sizeFromContents(
            QStyle::CT_PushButton,
            &opt,
            contents_size,
            nullptr
         );
         widths[i] = button_size.width();
      } else {
         widths[i] = button->sizeHint().width();
      }
      total_width += widths[i];
   }

   bool   any_truncated = false;
   size_t count_to_show = count;

   // clickable width to trigger a textbox
   constexpr const int gutter_size = 50;

   int available = this->size().width();
   if (this->allowTextEditing() && available > gutter_size * 2) {
      available -= gutter_size;
   }
   if (available < total_width) {
      if (available < widths.back()) {
         count_to_show = 1;
         widths.back() = available;
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
      size_t i = 0;
      for (; i < first_to_show; ++i) {
         auto* button = segments[i].button;
         button->setEnabled(false);
         button->setVisible(false);
      }
      int x = this->contentsMargins().left();
      int y = this->contentsMargins().top();
      for (; i < count; ++i) {
         auto* button = segments[i].button;
         button->setEnabled(true);
         button->setVisible(true);
         button->setGeometry(QRect(x, y, widths[i], height));
         x += widths[i];
      }
   }

   last_layout.any_truncated = any_truncated;
   last_layout.count_shown   = count_to_show;
}

#pragma region Events
   /*virtual*/ void DKBreadcrumbBar::mousePressEvent(QMouseEvent* event) {
      QWidget::mousePressEvent(event);
      if (event->button() == Qt::LeftButton) {
         //
         // TODO: Show the textbox.
         //
      }
   }
   /*virtual*/ void DKBreadcrumbBar::resizeEvent(QResizeEvent* event) {
      this->_re_layout();
   }
   /*virtual*/ void DKBreadcrumbBar::showEvent(QShowEvent* event) {
      this->_re_layout();
   }
#pragma endregion