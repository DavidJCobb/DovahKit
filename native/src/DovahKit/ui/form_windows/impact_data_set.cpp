#include "./impact_data_set.h"
#include <limits>
#include <QHeaderView>
#include "editor/form_stub_meta_type.h"
#include "ui/utils/bind.h"
#include "ui/utils/typical_tableview_config.h"
#include "widgets/DKHeaderView.h"
#include "./impact_data_set/ImpactDataSetContentsModel.h"

FormDialogImpactDataSet::FormDialogImpactDataSet(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->_model = new ImpactDataSetContentsModel(this);
   this->ui.view->setModel(this->_model);
   ui::typical_tableview_config(this->ui.view);
   {
      auto* header = new DKHeaderView(Qt::Orientation::Horizontal, this->ui.view);
      this->ui.view->setHorizontalHeader(header);

      header->setFlexResizeEnabled(true);
      header->setColumnFlex(ImpactDataSetContentsModel::Column::MaterialTypeName,   1, 1, 0);
      header->setColumnFlex(ImpactDataSetContentsModel::Column::MaterialTypeFormID, 0, 0, 4);
      header->setColumnFlex(ImpactDataSetContentsModel::Column::ImpactDataName,     1, 1, 0);
      header->setColumnFlex(ImpactDataSetContentsModel::Column::ImpactDataFormID,   0, 0, 4);
   }
   {
      auto* sel_model = this->ui.view->selectionModel();
      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, sel_model]() {
         auto* editor = this->ui.impactData;

         QModelIndex qmi;
         {
            auto rows = sel_model->selectedRows();
            if (rows.empty()) {
               editor->setEnabled(false);
               return;
            }
            editor->setEnabled(true);
            if (!rows.empty())
               qmi = rows[0];
         }
         auto* stub = this->_model->data(qmi.siblingAtColumn(ImpactDataSetContentsModel::Column::ImpactDataName), ImpactDataSetContentsModel::FormStubRole).value<dovah::form_stub*>();

         const auto blocker = QSignalBlocker(editor);
         editor->setFormStub(stub);
      });
      QObject::connect(this->ui.impactData, &DKFormPicker::formChanged, this, [this, sel_model](dovah::form_stub* v) {
         QModelIndex qmi;
         {
            auto rows = sel_model->selectedRows();
            if (!rows.empty())
               qmi = rows[0];
         }
         this->_model->setImpactData(qmi, v);
      });

   }

   this->ui.impactData->setAllowedFormType(dovah::form_type::impact_data);
   this->ui.setAllUnsetForm->setAllowedFormType(dovah::form_type::impact_data);

   QObject::connect(this->ui.setAllUnsetButton, &QPushButton::clicked, this, [this]() {
      this->_model->setAllUnsetTo(this->ui.setAllUnsetForm->formStub());
   });

   this->load(); // this creates the working copy.
}
void FormDialogImpactDataSet::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->_model->importData(working);
}
void FormDialogImpactDataSet::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->_model->exportData(working);
}