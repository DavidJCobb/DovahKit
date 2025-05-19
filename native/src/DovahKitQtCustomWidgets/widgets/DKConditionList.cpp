#include "./DKConditionList.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QTableView>
#if !defined(QT_PLUGIN)
   #include "./widget-dialogs/DKConditionEditDialog.h"
   #include "./widget-models/DKConditionListModel.h"
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
   this->setLayout(layout);
   layout->setMargin(0);

   {
      auto* view = this->_subwidgets.view = new QTableView(this);
      layout->addWidget(view, 1);
   }
   {
      button_wrap = new QWidget(this);
      layout->addWidget(button_wrap);

      auto* sublayout = new QHBoxLayout(button_wrap);
      button_wrap->setLayout(sublayout);
      {
         auto* button = this->_subwidgets.move_up = new QPushButton(tr("<<", "move up label"));
         sublayout->addWidget(button);
      }
      {
         auto* button = this->_subwidgets.move_down = new QPushButton(tr(">>", "move down label"));
         sublayout->addWidget(button);
      }
      sublayout->addStretch(1);
      {
         auto* button = this->_subwidgets.add_item = new QPushButton(tr("New", "add new condition label"));
         sublayout->addWidget(button);
      }
   }
   
   #pragma region Tab order
      this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      this->setFocusProxy(this->_subwidgets.view);
      this->setTabOrder(this->_subwidgets.view, button_wrap);
      
      button_wrap->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      button_wrap->setFocusProxy(this->_subwidgets.move_up);
      this->setTabOrder(this->_subwidgets.move_up,   this->_subwidgets.move_down);
      this->setTabOrder(this->_subwidgets.move_down, this->_subwidgets.add_item);
   #pragma endregion

   #if !defined(QT_PLUGIN)
   {
      auto* widget = this->_subwidgets.view;
      this->_model = new DKConditionListModel(this);
      widget->setModel(this->_model);
   }
   #endif
}

#if !defined(QT_PLUGIN)
   void DKConditionList::importFrom(dovah::loaded_forms::Form& owner, const BackendConditionList& target) {
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
   void DKConditionList::clear() {
      this->_model->clear();
      this->_owning_stub = nullptr;
   }
#endif

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