#include "./DKFormDestructionDataDialog.h"
#include <QKeyEvent>
#include "./DKFormDestructionStageDialog.h"
#include "../widget-models/DKFormDestructionStageListModel.h"
#include "../DKHeaderView.h"
#include "ui/utils/DKStyledItemDelegate.h"

DKFormDestructionDataDialog::DKFormDestructionDataDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   this->_model = new DKFormDestructionStageListModel(this);
   this->ui.stages->setModel(this->_model);

   // consistency with CK:
   this->ui.health->setMaximum(std::numeric_limits<int32_t>::max());

   {
      auto* view = this->ui.stages;
      view->installEventFilter(this); // for the Delete key
      if (auto* vh = view->verticalHeader()) {
         vh->setVisible(false);
         vh->setSectionResizeMode(QHeaderView::ResizeToContents);
      }

      auto* delegate = new DKStyledItemDelegate(view);
      view->setItemDelegate(delegate);
      
      QObject::connect(view, &QTableView::doubleClicked, this, [this](const QModelIndex& qmi) {
         auto  row = qmi.row();
         auto* src = this->_model->stage(row);
         if (!src)
            return;

         auto* dialog = new DKFormDestructionStageDialog(this);
         dialog->setValue(*src);
         auto  result = dialog->exec();
         dialog->deleteLater();
         if (result == QDialog::DialogCode::Rejected)
            return;

         auto new_qmi = this->_model->setStage(row, dialog->value());
         if (new_qmi.isValid())
            this->_selectRow(new_qmi);
      });
   }
   QObject::connect(this->ui.stageButtonAdd, &QPushButton::clicked, this, [this]() {
      auto* dialog = new DKFormDestructionStageDialog(this);
      auto  result = dialog->exec();
      dialog->deleteLater();
      if (result == QDialog::DialogCode::Rejected)
         return;

      auto qmi = this->_model->insertStage(dialog->value());
      if (qmi.isValid())
         this->_selectRow(qmi);
   });
   QObject::connect(this->ui.stageButtonEdit, &QPushButton::clicked, this, &DKFormDestructionDataDialog::_editSelectedStage);
   QObject::connect(this->ui.stageButtonDelete, &QPushButton::clicked, this, &DKFormDestructionDataDialog::_deleteSelectedStage);

   QObject::connect(this->ui.enabled, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.health->setEnabled(checked);
      this->ui.stages->setEnabled(checked);
      this->ui.flagVATSTargetable->setEnabled(checked);
      if (!checked) {
         this->_clearData();
      }
   });

   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
}

std::optional<DKFormDestructionDataDialog::DestructionData> DKFormDestructionDataDialog::data() const {
   if (!this->ui.enabled->isChecked())
      return {};

   std::optional<DestructionData> dst_opt;
   auto& dst = dst_opt.emplace();

   dst.health = this->ui.health->value();
   dst.flags.vats_enabled = this->ui.flagVATSTargetable->isChecked();

   size_t size = this->_model->rowCount();
   for (size_t i = 0; i < size; ++i) {
      auto* src = this->_model->stage(i);
      if (!src)
         continue;
      dst.stages.push_back(*src);
   }

   return dst_opt;
}
void DKFormDestructionDataDialog::setData(const std::optional<DestructionData>& src_opt) {
   this->ui.enabled->setChecked(src_opt.has_value());
   if (!src_opt.has_value()) {
      this->ui.health->setEnabled(false);
      this->ui.stages->setEnabled(false);
      this->ui.flagVATSTargetable->setEnabled(false);
      this->_clearData();
      return;
   }
   auto& src = src_opt.value();

   this->ui.health->setValue(src.health);
   this->ui.flagVATSTargetable->setChecked(src.flags.vats_enabled);
   this->_model->replaceStages(src.stages);
}

/*virtual*/ bool DKFormDestructionDataDialog::eventFilter(QObject* target, QEvent* event) /*override*/ {
   if (target != this->ui.stages)
      return false;

   if (event->type() == QEvent::KeyPress) {
      if (auto* casted = static_cast<QKeyEvent*>(event)) {
         if (!this->ui.stages->hasFocus())
            return false;

         this->_deleteSelectedStage();
         return true;
      }
   }
   return false;
}

void DKFormDestructionDataDialog::_deleteSelectedStage() {
   auto* sm = this->ui.stages->selectionModel();
   if (!sm)
      return;

   auto rows = sm->selectedRows();
   if (rows.empty())
      return;

   auto row = rows[0].row();

   this->_model->deleteItems(row, 1);
}
void DKFormDestructionDataDialog::_editSelectedStage() {
   auto* sm = this->ui.stages->selectionModel();
   if (!sm)
      return;

   auto rows = sm->selectedRows();
   if (rows.empty())
      return;

   auto  row = rows[0].row();
   auto* src = this->_model->stage(row);
   if (!src)
      return;

   auto* dialog = new DKFormDestructionStageDialog(this);
   dialog->setValue(*src);
   auto  result = dialog->exec();
   dialog->deleteLater();
   if (result == QDialog::DialogCode::Rejected)
      return;

   auto qmi = this->_model->setStage(row, dialog->value());
   if (qmi.isValid())
      this->_selectRow(qmi);
}

void DKFormDestructionDataDialog::_clearData() {
   this->ui.health->setValue(1);
   this->ui.flagVATSTargetable->setChecked(false);
   this->_model->clear();
}

void DKFormDestructionDataDialog::_selectRow(const QModelIndex& qmi) {
   if (auto* sm = this->ui.stages->selectionModel()) {
      auto lefthand  = qmi.siblingAtColumn(0);
      auto righthand = qmi.siblingAtColumn(this->_model->columnCount({}) - 1);
      sm->select(QItemSelection(lefthand, righthand), QItemSelectionModel::SelectionFlag::ClearAndSelect);
   }
}