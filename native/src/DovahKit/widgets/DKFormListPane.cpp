#include "DKFormListPane.h"
#include <QBoxLayout>
#include <QFontMetrics>
#include <QHeaderView>
#include <QKeyEvent>
#include "DKHeaderView.h"
#if !defined(QT_DESIGNER_LIB)
   #include "../editor/open_window_for_form.h"
   #include "./widget-data/DKCustomFormFilter.h"
#endif

namespace {
   constexpr QSize no_maximum_size = { QWIDGETSIZE_MAX, QWIDGETSIZE_MAX };
}

#if defined(QT_DESIGNER_LIB)
#include <QAbstractItemModel>
class DKFormListPaneModel : public QAbstractItemModel {
   public:
      using QAbstractItemModel::QAbstractItemModel;

      virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override { return QModelIndex(); }
      virtual QModelIndex parent(const QModelIndex& index) const { return QModelIndex(); }
      virtual int rowCount(const QModelIndex& parent) const override { return 0; }
      virtual int columnCount(const QModelIndex& item) const override { return 3; }
      virtual Qt::ItemFlags flags(const QModelIndex& index) const override { return 0; }
      virtual QVariant data(const QModelIndex& index, int role) const override { return QVariant(); }
      virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
         if (orientation != Qt::Orientation::Horizontal) {
            return QVariant();
         }
         switch (role) {
            case Qt::DisplayRole:
               switch (section) {
                  case DKFormListPane::ColumnType:   return DKFormListPane::tr("Type", "FormList listview");
                  case DKFormListPane::ColumnName:   return DKFormListPane::tr("Name", "FormList listview");
                  case DKFormListPane::ColumnFormID: return DKFormListPane::tr("Form ID", "FormList listview");
               }
               break;
         }
         return QVariant();
      }
};
#endif

DKFormListPane::DKFormListPane(QWidget* parent) : QWidget(parent) {
   auto* view = this->subwidgets.view = new QTableView(this);
   auto* wrap = this->subwidgets.buttons.wrapper = new QWidget(this);
   this->subwidgets.buttons.move_down = new QPushButton(wrap);
   this->subwidgets.buttons.move_up   = new QPushButton(wrap);
   this->subwidgets.buttons.remove    = new QPushButton(wrap);
   this->subwidgets.buttons.remove->setText(tr("Remove"));
   //
   {
      auto* layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight, this);
      auto* nested = new QBoxLayout(QBoxLayout::Direction::TopToBottom, wrap);
      layout->addWidget(view, 1);
      layout->addWidget(wrap, 0);
      nested->addStretch(1);
      nested->addWidget(this->subwidgets.buttons.move_up);
      nested->addWidget(this->subwidgets.buttons.move_down);
      nested->addWidget(this->subwidgets.buttons.remove);
      nested->addStretch(1);
      //
      layout->setContentsMargins({ 0, 0, 0, 0 });
      nested->setContentsMargins({ 0, 0, 0, 0 });
      layout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
      nested->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
   }
   #pragma region Tab order
      this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      this->setFocusProxy(this->subwidgets.view);
      this->setTabOrder(this->subwidgets.view, this->subwidgets.buttons.wrapper);
      //
      this->subwidgets.buttons.wrapper->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      this->subwidgets.buttons.wrapper->setFocusProxy(this->subwidgets.buttons.move_up);
      this->setTabOrder(this->subwidgets.buttons.move_up,   this->subwidgets.buttons.move_down);
      this->setTabOrder(this->subwidgets.buttons.move_down, this->subwidgets.buttons.remove);
   #pragma endregion
   //
   view->setModel(new DKFormListPaneModel(this));
   #if !defined(QT_DESIGNER_LIB)
      QObject::connect(view, &QTableView::doubleClicked, [this, view](const QModelIndex& index) {
         if (!index.isValid())
            return;
         auto* model = (DKFormListPaneModel*)view->model();
         auto  data  = (DKFormListPaneModel::Item*)index.internalPointer();
         if (data && data->stub)
            open_edit_dialog_for_form(*(data->stub), this);
      });
   #endif
   view->setSelectionBehavior(QAbstractItemView::SelectRows);
   view->setSelectionMode(this->allowMultiSelect() ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
   view->setCornerButtonEnabled(false);
   view->setAcceptDrops(true);
   view->setDragDropOverwriteMode(false);
   {
      auto* header = new DKHeaderView(Qt::Horizontal, view);
      header->setFlexResizeEnabled(true);
      view->setHorizontalHeader(header);
      //
      auto metrics = QFontMetrics(view->font());
      header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
      header->setMinimumSectionSize(2);
      header->setColumnFlex(ColumnType,   0, 0, metrics.boundingRect("XMMX").width() * 1.5F + 4);
      header->setColumnFlex(ColumnName,   5, 0); // large flex grow factor so that extra columns (which have a default flex of 1) aren't prioritized equally by default
      header->setColumnFlex(ColumnFormID, 0, 0, metrics.boundingRect("00000000").width() * 1.5F + 4); // sets minimum size
      header->modSectionSizeTo(ColumnFormID, 4); // mimics a user resize and shrinks the column
      header->setSectionResizeMode(ColumnType,   QHeaderView::Interactive);
      header->setSectionResizeMode(ColumnName,   QHeaderView::Interactive);
      header->setSectionResizeMode(ColumnFormID, QHeaderView::Interactive);
      header->setStretchLastSection(false);
      //
      view->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
      view->verticalHeader()->setVisible(this->state.show_indices);
      #if !defined(QT_DESIGNER_LIB)
      {
         auto* model = this->_model();
         model->setAllowDuplicates(this->state.allow_duplicates);
         model->setShowIndices(this->state.show_indices);

         QObject::connect(model, &DKFormListPaneModel::rowsInserted, this, [this](const QModelIndex&, int first, int last) {
            emit this->formsAdded(last - first + 1);
         });
         QObject::connect(model, &DKFormListPaneModel::rowsRemoved, this, [this](const QModelIndex&, int first, int last) {
            emit this->formsRemoved(last - first + 1);
         });
      }
      #endif
   }
   //
   #if !defined(QT_DESIGNER_LIB)
      QObject::connect(this->subwidgets.buttons.move_up,   &QPushButton::clicked, this, [this]() { this->_moveSelected(-1); });
      QObject::connect(this->subwidgets.buttons.move_down, &QPushButton::clicked, this, [this]() { this->_moveSelected(1); });
      QObject::connect(this->subwidgets.buttons.remove,    &QPushButton::clicked, this, [this]() { this->_removeSelected(); });
   #endif
   #if !defined(QT_DESIGNER_LIB)
      {
         auto* sel_model = this->subwidgets.view->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection& sel) {
            if (sel.isEmpty()) {
               emit selectedRowsChanged({});
               emit selectedFormsChanged({});
               return;
            }

            std::vector<dovah::form_stub*> forms;
            std::vector<size_t> rows;

            for (auto& sel_item : sel) {
               int first = sel_item.top();
               int last  = sel_item.bottom();
               for (int i = first; i <= last; ++i) {
                  forms.push_back(this->_model()->getNthStub(i));
                  rows.push_back(i);
               }
            }
            emit selectedRowsChanged(rows);
            emit selectedFormsChanged(forms);
         });
      }
   #endif
   //
   this->_updateOrientation();
}

DKFormListPaneModel* DKFormListPane::_model() const noexcept {
   return (DKFormListPaneModel*) this->subwidgets.view->model();
}

#if !defined(QT_DESIGNER_LIB)
   void DKFormListPane::_moveSelected(int down) {
      if (this->readOnly())
         return;
      auto* sm = this->subwidgets.view->selectionModel();
      if (!sm)
         return;
      this->_model()->moveStubs(sm->selectedRows(), down);
   }
   void DKFormListPane::_removeSelected() {
      if (this->readOnly())
         return;
      auto* sm = this->subwidgets.view->selectionModel();
      if (!sm)
         return;
      this->_model()->removeStubs(sm->selectedRows());
   }
#endif
void DKFormListPane::_updateButtonVisibility() {
   if (this->readOnly()) {
      this->subwidgets.buttons.wrapper->setVisible(false);
      return;
   }
   auto* prev   = this->subwidgets.buttons.move_up;
   auto* next   = this->subwidgets.buttons.move_down;
   auto* remove = this->subwidgets.buttons.remove;
   bool mv = this->state.show_move_buttons;
   bool rv = this->state.show_remove_button;
   prev->setVisible(mv);
   next->setVisible(mv);
   remove->setVisible(rv);
   this->subwidgets.buttons.wrapper->setVisible(mv || rv);
}
void DKFormListPane::_updateOrientation() {
   auto* prev   = this->subwidgets.buttons.move_up;
   auto* next   = this->subwidgets.buttons.move_down;
   auto* layout = (QBoxLayout*)this->layout();
   auto* nested = (QBoxLayout*)this->subwidgets.buttons.wrapper->layout();
   assert(layout && nested);
   auto  count  = nested->count();
   assert(count > 0);
   if (this->orientation() == Qt::Orientation::Vertical) { // orientation of the "main axis;" buttons are laid out along the "cross axis"
      prev->setText(tr("<<"));
      next->setText(tr(">>"));
      {
         prev->ensurePolished();
         if (auto* style = prev->style()) {
            auto fm    = QFontMetrics(prev->font());
            auto width = fm.boundingRect("<<").width();
            //
            QStyleOptionButton opt;
            opt.initFrom(prev);
            opt.text = "<<";
            auto frame = style->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, prev);
            frame += style->pixelMetric(QStyle::PM_ButtonMargin, &opt, prev);
            frame *= 2;
            //
            prev->setMaximumSize({ frame + width, QWIDGETSIZE_MAX });
            next->setMaximumSize({ frame + width, QWIDGETSIZE_MAX });
         }
      }
      nested->setDirection(QBoxLayout::Direction::LeftToRight);
      layout->setDirection(QBoxLayout::Direction::TopToBottom);
      nested->setStretch(0,         1); // stretch on both sides; center buttons
      nested->setStretch(count - 1, 1); // stretch on both sides; center buttons
   } else {
      prev->setText(tr("Move Up"));
      next->setText(tr("Move Down"));
      prev->setMaximumSize(no_maximum_size);
      next->setMaximumSize(no_maximum_size);
      nested->setDirection(QBoxLayout::Direction::TopToBottom);
      layout->setDirection(QBoxLayout::Direction::LeftToRight);
      nested->setStretch(0, 0);         // no stretch at start
      nested->setStretch(count - 1, 1); // stretch at end; push buttons to top
   }
}

#if !defined(QT_DESIGNER_LIB)
   QVector<dovah::form_stub*> DKFormListPane::stubs() const noexcept {
      return this->_model()->stubs();
   }
   void DKFormListPane::pullStubs(const std::vector<dovah::form_reference_t>& list) {
      this->clear();
      this->reserve(list.size());
      for (auto& ref : list)
         this->addStub(ref.get_form_stub());
   }
   void DKFormListPane::commitStubs(std::vector<dovah::form_reference_t>& list, dovah::loaded_forms::Form& owner) {
      auto   stubs = this->stubs();
      size_t i     = 0;
      size_t size  = stubs.size();
      if (list.size() < size)
         list.resize(size);
      for (; i < size; ++i)
         list[i].set(owner, stubs[i]);
      //
      // Delete excess elements, if any were removed:
      //
      auto s = list.size();
      if (s != size) {
         for (; i < s; ++i)
            list[i].set(owner, nullptr);
         list.resize(size);
      }
   }
#endif

void DKFormListPane::setAllowDuplicates(bool v) {
   this->state.allow_duplicates = v;
   #if !defined(QT_DESIGNER_LIB)
      this->_model()->setAllowDuplicates(v);
   #endif
}
void DKFormListPane::setAllowMultiSelect(bool v) {
   auto& dst = this->state.allow_multi_select;
   if (v == dst)
      return;
   dst = v;
   this->subwidgets.view->setSelectionMode(v ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
}
void DKFormListPane::setReadOnly(bool v) {
   if (this->readOnly() == v)
      return;
   this->state.read_only = v;

   auto* view = this->subwidgets.view;
   view->setAcceptDrops(!v);
   this->subwidgets.buttons.wrapper->setEnabled(!v);
   this->subwidgets.buttons.wrapper->setVisible(!v);
}
#if !defined(QT_DESIGNER_LIB)
   void DKFormListPane::setAllowedFormTypes(QVector<dovah::form_type> list) {
      this->_model()->setAllowedFormTypes(list);
   }
#endif
void DKFormListPane::setShowFormTypes(bool v) {
   if (v == this->state.show_form_types)
      return;
   this->state.show_form_types = v;
   this->subwidgets.view->setColumnHidden(ColumnType, !v);
}
void DKFormListPane::setShowIndices(bool v) {
   if (v == this->state.show_indices)
      return;
   this->state.show_indices = v;
   this->subwidgets.view->verticalHeader()->setVisible(v);
   #if !defined(QT_DESIGNER_LIB)
      this->_model()->setShowIndices(v);
   #endif
}
void DKFormListPane::setOrientation(Qt::Orientation o) {
   if (this->orientation() == o)
      return;
   this->state.orientation = o;
   this->_updateOrientation();
}
void DKFormListPane::setShowMoveButtons(bool v) {
   if (v == this->state.show_move_buttons)
      return;
   this->state.show_move_buttons = v;
   this->_updateButtonVisibility();
}
void DKFormListPane::setShowRemoveButton(bool v) {
   if (v == this->state.show_remove_button)
      return;
   this->state.show_remove_button = v;
   this->_updateButtonVisibility();
}

#if !defined(QT_DESIGNER_LIB)
   void DKFormListPane::addExtraColumn(QString header, ExtraColumnHandler&& handler) {
      this->_model()->addExtraColumn(header, std::forward<ExtraColumnHandler>(handler));
   }
   void DKFormListPane::removeExtraColumn(size_t which) {
      this->_model()->removeExtraColumn(which);
   }
#endif
   

#if !defined(QT_DESIGNER_LIB)
   DKCustomFormFilter* DKFormListPane::customFilter() const {
      return this->_model()->get_custom_filter();
   }
   void DKFormListPane::setCustomFilter(DKCustomFormFilter* v) {
      this->_model()->set_custom_filter(v);
   }
#endif
   

#if !defined(QT_DESIGNER_LIB)
   [[nodiscard]] std::vector<dovah::form_stub*> DKFormListPane::selectedForms() const {
      std::vector<dovah::form_stub*> forms;

      auto sel = this->subwidgets.view->selectionModel()->selection();
      for (auto& sel_item : sel) {
         int first = sel_item.top();
         int last  = sel_item.bottom();
         for (int i = first; i <= last; ++i) {
            forms.push_back(this->_model()->getNthStub(i));
         }
      }

      return forms;
   }
   [[nodiscard]] std::vector<size_t> DKFormListPane::selectedRows() const {
      std::vector<size_t> rows;

      auto sel = this->subwidgets.view->selectionModel()->selection();
      for (auto& sel_item : sel) {
         int first = sel_item.top();
         int last  = sel_item.bottom();
         for (int i = first; i <= last; ++i) {
            rows.push_back(i);
         }
      }

      return rows;
   }
#endif

#if !defined(QT_DESIGNER_LIB)
   void DKFormListPane::addStub(dovah::form_stub* stub) {
      this->_model()->addStub(stub);
   }
   void DKFormListPane::clear() {
      this->_model()->clear();
   }
   bool DKFormListPane::contains(const dovah::form_stub* stub) const {
      return this->indexOf(stub) >= 0;
   }
   int DKFormListPane::indexOf(const dovah::form_stub* stub) const {
      return this->_model()->indexOfStub(stub);
   }
   void DKFormListPane::reserve(size_t i) {
      this->_model()->reserve(i);
   }
#endif

void DKFormListPane::keyPressEvent(QKeyEvent* event) {
   #if !defined(QT_DESIGNER_LIB)
      if (!this->readOnly()) {
         if (event->matches(QKeySequence::Delete)) {
            this->_removeSelected();
         }
      }
   #endif
}