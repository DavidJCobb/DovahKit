#include "./DKFormDestructionDataDialog.h"
#include "./DKFormDestructionStageDialog.h"
#include "../widget-models/DKFormDestructionStageListModel.h"

DKFormDestructionDataDialog::DKFormDestructionDataDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   this->_model = new DKFormDestructionStageListModel(this);
   this->ui.stages->setModel(this->_model);

   // consistency with CK:
   this->ui.health->setMaximum(std::numeric_limits<int32_t>::max());

   {
      auto* view = this->ui.stages;
      static_assert(false, "TODO: config table");
      static_assert(false, "TODO: DKHeaderView and col widths");

      static_assert(false, "TODO: double-click handler");
      static_assert(false, "TODO: delete key handler");
   }
   QObject::connect(this->ui.stageButtonAdd, &QPushButton::clicked, this, [this]() {
      auto* dialog = new DKFormDestructionStageDialog(this);
      auto  result = dialog->exec();
      dialog->deleteLater();
      if (result == QDialog::DialogCode::Rejected)
         return;

      auto qmi = this->_model->appendStage(dialog->value());
      if (auto* sm = this->ui.stages->selectionModel()) {
         auto righthand = qmi.siblingAtColumn(this->_model->columnCount({}) - 1);
         sm->select(QItemSelection(qmi, righthand), QItemSelectionModel::SelectionFlag::ClearAndSelect);
      }
   });
   QObject::connect(this->ui.stageButtonEdit, &QPushButton::clicked, this, &DKFormDestructionDataDialog::_editSelectedStage);
   QObject::connect(this->ui.stageButtonDelete, &QPushButton::clicked, this, &DKFormDestructionDataDialog::_deleteSelectedStage);

   QObject::connect(this->ui.enabled, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.health->setEnabled(checked);
      this->ui.stages->setEnabled(checked);
      this->ui.flagVATSTargetable->setEnabled(checked);
      if (!checked) {
         this->ui.health->setValue(1);
         this->ui.flagVATSTargetable->setChecked(false);
         this->_model->clear();
      }
   });
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
   this->ui.enabled->setChecked(src_opt.has_value()); // trigger own signal handler on purpose
   if (!src_opt.has_value()) {
      return;
   }
   auto& src = src_opt.value();

   this->ui.health->setValue(src.health);
   this->ui.flagVATSTargetable->setChecked(src.flags.vats_enabled);
   this->_model->replaceStages(src.stages);
}

void DKFormDestructionDataDialog::_deleteSelectedStage();
void DKFormDestructionDataDialog::_editSelectedStage();