#include "./DKConditionList.h"
#include <QBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QPushButton>
#include <QTableView>
#if !defined(QT_PLUGIN)
   #include "./widget-dialogs/DKConditionEditDialog.h"
   #include "./widget-models/DKConditionListModel.h"
   #include "./DKHeaderView.h"

   #include "dovah/data/conditions/all_function_info.h"
   #include "dovah/forms/Form.h"
   #include "helpers/string/strieq_ascii.h"
   #include "ui/types/conditions/condition.h"
#endif

namespace {
   #if !defined(QT_PLUGIN)
   constexpr const auto GetIsID = []() -> uint16_t {
      for (auto& func : dovah::conditions::all_vanilla_function_info) {
         if (cobb::strieq_ascii(func.name, "GetIsID"))
            return func.id;
      }
      return -1;
   }();
   #endif
}

DKConditionList::DKConditionList(QWidget* parent) : QWidget(parent) {
   auto*    layout      = new QVBoxLayout(this);
   QWidget* button_wrap = nullptr;
   layout->setContentsMargins(0, 0, 0, 0);
   this->setLayout(layout);

   {
      auto* view = this->_subwidgets.view = new QTableView(this);
      layout->addWidget(view, 1);

      if (auto* vh = view->verticalHeader()) {
         vh->setVisible(false);
         vh->setSectionResizeMode(QHeaderView::ResizeToContents);
      }
      view->setCornerButtonEnabled(false);
      view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      view->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
      view->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
      view->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
      view->setWordWrap(false);
   }
   {
      button_wrap = new QWidget(this);
      layout->addWidget(button_wrap);

      auto* sublayout = new QHBoxLayout(button_wrap);
      sublayout->setContentsMargins(0, 0, 0, 0);
      button_wrap->setLayout(sublayout);
      {
         auto* button = this->_subwidgets.move_up = new QPushButton(tr("<<", "move up label"));
         button->setAccessibleName(tr("Move Up"));
         button->setAccessibleDescription(tr("Moves the selected conditions up within the list."));
         sublayout->addWidget(button);
      }
      {
         auto* button = this->_subwidgets.move_down = new QPushButton(tr(">>", "move down label"));
         button->setAccessibleName(tr("Move Down"));
         button->setAccessibleDescription(tr("Moves the selected conditions down within the list."));
         sublayout->addWidget(button);
      }
      {
         auto* button = this->_subwidgets.remove_item = new QPushButton(tr("Delete"));
         button->setAccessibleDescription(tr("Deletes the selected conditions."));
         sublayout->addWidget(button);
      }
      sublayout->addStretch(1);
      {
         auto* button = this->_subwidgets.add_item = new QPushButton(tr("New", "add new condition label"));
         button->setAccessibleDescription(tr("Adds a new condition to the end of the list."));
         sublayout->addWidget(button);
      }
   }
   
   #pragma region Tab order
      this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      this->setFocusProxy(this->_subwidgets.view);
      this->setTabOrder(this->_subwidgets.view, button_wrap);
      
      button_wrap->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      button_wrap->setFocusProxy(this->_subwidgets.move_up);
      this->setTabOrder(this->_subwidgets.move_up,     this->_subwidgets.move_down);
      this->setTabOrder(this->_subwidgets.move_down,   this->_subwidgets.remove_item);
      this->setTabOrder(this->_subwidgets.remove_item, this->_subwidgets.add_item);
   #pragma endregion

   #if !defined(QT_PLUGIN)
      {
         auto* widget = this->_subwidgets.view;
         this->_model = new DKConditionListModel(this);
         widget->setModel(this->_model);

         {  // Set up new header
            //
            // Have to do this after setting the model, because QHeaderView::setSectionResizeMode 
            // and friends will crash if the section in question doesn't exist yet.
            //
            auto* header = new DKHeaderView(Qt::Orientation::Horizontal, widget);
            widget->setHorizontalHeader(header);
            header->setFlexResizeEnabled(true);

            header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
            header->setMinimumSectionSize(2);
            {
               using Column = DKConditionListModel::Column;

               auto metrics = widget->fontMetrics();

               header->setColumnFlex(Column::Target,   0, 0, metrics.boundingRect("Subject 123").width() * 1.5F + 4);
               header->setColumnFlex(Column::Function, 2, 2, metrics.boundingRect("LongIshFunctionName").width() * 1.5F + 4);
               header->setColumnFlex(Column::Args,     4, 1);
               header->setColumnFlex(Column::Operator, 0, 0, metrics.boundingRect("==").width() * 1.5F + 4);
               header->setColumnFlex(Column::Operand,  1, 3, metrics.boundingRect("12345.6789").width() * 1.5F + 4);
               header->setColumnFlex(Column::UsesOr,   0, 0, metrics.boundingRect("AND").width() * 1.5F + 4);

               header->setSectionResizeMode(Column::Target,   QHeaderView::Interactive);
               header->setSectionResizeMode(Column::Function, QHeaderView::Interactive);
               header->setSectionResizeMode(Column::Args,     QHeaderView::Interactive);
               header->setSectionResizeMode(Column::Operand,  QHeaderView::Interactive);
            }
            header->setStretchLastSection(false);
         }
      }
      QObject::connect(this->_subwidgets.add_item, &QPushButton::clicked, this, &DKConditionList::openCreateConditionModal);
      QObject::connect(this->_subwidgets.view, &QTableView::doubleClicked, this, [this](const QModelIndex& qmi) {
         auto start = qmi.siblingAtColumn(0);
         auto end   = qmi.siblingAtColumn(DKConditionListModel::column_count - 1);

         auto* sm = this->_subwidgets.view->selectionModel();
         sm->select(QItemSelection(start, end), QItemSelectionModel::SelectionFlag::ClearAndSelect);

         this->openEditConditionModal();
      });

      QObject::connect(this->_subwidgets.move_up,     &QPushButton::clicked, this, &DKConditionList::_move_selection_up);
      QObject::connect(this->_subwidgets.move_down,   &QPushButton::clicked, this, &DKConditionList::_move_selection_down);
      QObject::connect(this->_subwidgets.remove_item, &QPushButton::clicked, this, &DKConditionList::_delete_selection);

      this->_subwidgets.view->installEventFilter(this); // Del key

      #pragma region Context menu
      {
         auto& menu    = this->_context.menu;
         auto& actions = this->_context.actions;
         {
            auto* action = actions.edit = new QAction(tr("Edit..."), &menu);
            menu.addAction(action);
            QObject::connect(action, &QAction::triggered, this, &DKConditionList::openEditConditionModal);
         }
         {
            auto* action = actions.move_up = new QAction(tr("Move Up"), &menu);
            menu.addAction(action);
            QObject::connect(action, &QAction::triggered, this, &DKConditionList::_move_selection_up);
         }
         {
            auto* action = actions.move_down = new QAction(tr("Move Down"), &menu);
            menu.addAction(action);
            QObject::connect(action, &QAction::triggered, this, &DKConditionList::_move_selection_down);
         }
         {
            auto* action = actions.remove = new QAction(tr("Delete"), &menu);
            menu.addAction(action);
            QObject::connect(action, &QAction::triggered, this, &DKConditionList::_delete_selection);
         }

         auto* widget = this->_subwidgets.view;
         widget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
         QObject::connect(widget, &QWidget::customContextMenuRequested, this, [this, widget, &menu](const QPoint& pos) {
            if (!this->_has_selection())
               return;
            menu.exec(widget->mapToGlobal(pos));
         });
      }
      #pragma endregion
   #endif

   QObject::connect(this->_subwidgets.view->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]() {
      this->_update_button_enable_states();
   });
   this->_update_button_enable_states();
}

#if !defined(QT_PLUGIN)
   void DKConditionList::importFrom(dovah::loaded_forms::Form& owner, const BackendConditionList& target) {
      this->_owning_stub = &owner.stub;
      this->_model->importFrom(owner, target);
   }
   void DKConditionList::importFrom(dovah::loaded_forms::Form& owner, const std::vector<ui::types::conditions::condition>& target) {
      this->_owning_stub = &owner.stub;
      this->_model->importFrom(owner, target);
   }
   void DKConditionList::exportTo(dovah::loaded_forms::Form& owner, BackendConditionList& dst) {
      auto src = this->_model->conditions();
      dst.clear(owner);
      for (auto& src_item : src) {
         auto& dst_item = dst.emplace_back();
         dst_item.commit(owner, src_item);
      }
   }
   void DKConditionList::exportTo(dovah::loaded_forms::Form& owner, std::vector<ui::types::conditions::condition>& dst) {
      dst = this->_model->conditions();
   }
   void DKConditionList::clear() {
      this->_model->clear();
      this->_owning_stub = nullptr;
   }

   void DKConditionList::overrideOwningForm(dovah::loaded_forms::Form& form) {
      this->_model->overrideOwningForm(form);
   }

   void DKConditionList::importBifurcatedList(dovah::loaded_forms::Form& owner, const BackendConditionList& locked, const BackendConditionList& normal) {
      this->_owning_stub = &owner.stub;
      this->_model->importBifurcatedList(owner, locked, normal);
   }
   void DKConditionList::exportBifurcatedList(dovah::loaded_forms::Form& owner, BackendConditionList& locked, BackendConditionList& normal) {
      this->_model->exportBifurcatedList(owner, locked, normal);
   }
#endif

size_t DKConditionList::conditionCount() const {
   #if !defined(QT_PLUGIN)
      return this->_model->rowCount();
   #else
      return 0;
   #endif
}

/*virtual*/ bool DKConditionList::eventFilter(QObject* object, QEvent* event) /*override*/ {
   #if !defined(QT_PLUGIN)
      if (object == this->_subwidgets.view) {
         if (event->type() == QEvent::Type::KeyPress) {
            if (((QKeyEvent*)event)->key() == Qt::Key_Delete) {
               auto* sm = this->_subwidgets.view->selectionModel();
               auto  sel = sm->selection();
               if (!sel.empty()) {
                  this->_delete_selection();
                  return true;
               }
            }
         }
      }
   #endif
   return false;
}

void DKConditionList::openCreateConditionModal() {
   #if !defined(QT_PLUGIN)
   auto* model = this->_model;
   auto* sm    = this->_subwidgets.view->selectionModel();
   if (!this->_owning_stub || !model || !sm)
      return;
   size_t insert_at = std::numeric_limits<size_t>::max();
   auto   rows      = sm->selectedRows();
   if (!rows.isEmpty())
      insert_at = rows.back().row() + 1;
   {
      ui::types::conditions::condition created;
      created.function           = GetIsID;
      created.run_on.type        = ui::types::conditions::run_on_type::subject;
      created.comparison.op      = ui::types::conditions::comparison_operator::equal;
      created.comparison.operand = 1.0F;
      created.reset_parameters();

      auto* modal = new DKConditionEditDialog(*this->_owning_stub, created, this);
      QObject::connect(modal, &QDialog::accepted, this, [this, model, modal, insert_at]() {
         auto  qmi = model->insertAt(modal->value(), insert_at);
         auto* sm  = this->_subwidgets.view->selectionModel();
         if (!sm)
            return;
         QModelIndex    dummy;
         QModelIndex    br = model->index(qmi.row(), model->columnCount(dummy) - 1, dummy); // (qmi) is just the left "edge" of the selection, and we want to select the whole row
         QItemSelection range(qmi, br);
         sm->select(range, QItemSelectionModel::ClearAndSelect);
         emit this->changeAttempted();
         emit this->changed();
      });
      QObject::connect(modal, &QDialog::finished, modal, &QObject::deleteLater);
      modal->setWindowModality(Qt::WindowModality::WindowModal);
      modal->open();
   }
   #endif
}
void DKConditionList::openEditConditionModal() {
   #if !defined(QT_PLUGIN)
   auto* model = this->_model;
   auto* sm    = this->_subwidgets.view->selectionModel();
   if (!this->_owning_stub || !model || !sm)
      return;
   auto  rows = sm->selectedRows();
   if (rows.size() != 1)
      return;
   auto  row  = rows[0].row();
   auto* cnd  = model->getCondition(row);
   if (!cnd)
      return;
   {
      auto* modal = new DKConditionEditDialog(*this->_owning_stub, *cnd, this);
      QObject::connect(modal, &QDialog::accepted, this, [this, model, modal, row, cnd]() {
         bool       changed = false;
         const auto after   = modal->value();
         if (auto* prior = model->getCondition(row)) {
            changed = *prior == after;
         }
         model->setCondition(row, after);
         emit this->changeAttempted();
         if (changed)
            emit this->changed();
      });
      QObject::connect(modal, &QDialog::finished, modal, &QObject::deleteLater);
      modal->setWindowModality(Qt::WindowModality::WindowModal);
      modal->open();
   }
   #endif
}

bool DKConditionList::_has_selection() const {
   #if defined(QT_PLUGIN)
      return false;
   #else
      auto* sm = this->_subwidgets.view->selectionModel();
      auto  sel = sm->selection();
      return !sel.empty();
   #endif
}
void DKConditionList::_update_button_enable_states() {
   bool enabled = this->_has_selection();
   this->_subwidgets.move_up->setEnabled(enabled);
   this->_subwidgets.move_down->setEnabled(enabled);
   this->_subwidgets.remove_item->setEnabled(enabled);
}

#if !defined(QT_PLUGIN)
   void DKConditionList::_move_selection_up() {
      auto* sm  = this->_subwidgets.view->selectionModel();
      auto  sel = sm->selection();
      if (sel.empty())
         return;
      this->_model->move(sel, -1);
   }
   void DKConditionList::_move_selection_down() {
      auto* sm  = this->_subwidgets.view->selectionModel();
      auto  sel = sm->selection();
      if (sel.empty())
         return;
      this->_model->move(sel, 1);
   }
   void DKConditionList::_delete_selection() {
      auto* sm = this->_subwidgets.view->selectionModel();
      auto  sel = sm->selection();
      if (sel.empty())
         return;
      this->_model->remove(sel);
   }
#endif