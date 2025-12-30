#include "./linked_refs.h"
#include <array>
#include <QGroupBox>
#include <QPushButton>
#include <QTableView>
#include <QVariant>
#include "helpers/bound_mem_fn.h"
#include "widgets/DKCompactObjectReferencePicker.h"
#include "widgets/DKFormPicker.h"
#include "dovah/forms/ObjectReference.h"
#include "editor/form_stub_meta_type.h"
#include "../ObjectReferenceLinkedRefsModel.h"
#include "../ObjectReferenceNewLinkedRefDialog.h"
#include "ui/utils/typical_tableview_config.h"

namespace ui::reference::fragments {
   void linked_refs::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;
      this->model    = new ObjectReferenceLinkedRefsModel(controls.view);
      this->owner    = &owner;
      
      controls.edit.keyword->setAllowedFormType(dovah::form_type::keyword);
      {
         auto* listview = this->controls.view;
         this->model = new model_type(listview);
         listview->setModel(this->model);
         ui::typical_tableview_config(listview);

         QObject::connect(listview->selectionModel(), &QItemSelectionModel::selectionChanged, &owner, cobb__bound_this_fn(_on_selection_changed));

         QObject::connect(this->controls.buttons.add,    &QPushButton::clicked, &owner, cobb__bound_this_fn(_try_add_link));
         QObject::connect(this->controls.buttons.remove, &QPushButton::clicked, &owner, cobb__bound_this_fn(_remove_selected_link));
         
         auto on_changed = cobb__bound_this_fn(_update_selected_link);
         QObject::connect(this->controls.edit.ref,     &DKCompactObjectReferencePicker::refChanged, &owner, on_changed);
         QObject::connect(this->controls.edit.keyword, &DKFormPicker::formChanged, &owner, on_changed);
      }
      controls.edit.ref->setValidationFunction([this](dovah::form_stub* ref) -> bool {
         return ref != this->stub;
      });
   }
   void linked_refs::load(loaded_form_type& form) {
      this->stub = &form.stub;

      this->model->importData(form);
   }
   void linked_refs::save(loaded_form_type& form) {
      this->model->exportData(form);
   }

   void linked_refs::_on_selection_changed(const QItemSelection& sel) {
      bool no_selection = sel.empty();
      this->controls.edit.groupbox->setDisabled(no_selection);
      this->controls.buttons.remove->setDisabled(no_selection);
      if (no_selection)
         return;
      QModelIndex qmi = sel[0].topLeft();

      const auto blockers = std::array{
         QSignalBlocker(this->controls.edit.keyword),
         QSignalBlocker(this->controls.edit.ref),
      };
      this->controls.edit.keyword->setFormStub(qmi.data(model_type::KeywordRole).value<dovah::form_stub*>());
      this->controls.edit.ref->setRef(qmi.data(model_type::RefRole).value<dovah::form_stub*>());
   }
   void linked_refs::_try_add_link() {
      auto* dialog = new ObjectReferenceNewLinkedRefDialog(this->owner);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      dialog->setDisallowedKeywords(this->model->allKeywords());
      if (this->stub)
         dialog->setDisallowedRef(*this->stub);
      if (dialog->exec() == QDialog::Accepted) {
         auto* keyword = dialog->keyword();
         auto* ref = dialog->ref();
         if (ref)
            this->model->setLink(keyword, ref);
      }
   }
   void linked_refs::_remove_selected_link() {
      auto* sel_model = this->controls.view->selectionModel();
      auto  sel       = sel_model->selectedRows();
      if (sel.empty())
         return;
      auto  qmi = sel[0];
      auto* kywd = qmi.data(model_type::KeywordRole).value<dovah::form_stub*>();
      this->model->setLink(kywd, nullptr);
   }
   void linked_refs::_update_selected_link() {
      auto* sel_model = this->controls.view->selectionModel();
      auto  sel       = sel_model->selectedRows();
      if (sel.empty())
         return;
      if (auto* ref = this->controls.edit.ref->ref())
         this->model->setRow(sel[0].row(), this->controls.edit.keyword->formStub(), ref);
   }
}