#include "./activate_parents.h"
#include <array>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QPushButton>
#include <QTableView>
#include <QVariant>
#include "helpers/bound_mem_fn.h"
#include "widgets/widget-dialogs/DKCompactObjectReferencePickerDialog.h"
#include "widgets/DKCompactObjectReferencePicker.h"
#include "widgets/DKHeaderView.h"
#include "dovah/forms/components/extra_data/types/a/activate_parents.h"
#include "dovah/forms/ObjectReference.h"
#include "editor/form_stub_meta_type.h"
#include "../ObjectReferenceActivateParentsModel.h"
#include "ui/utils/set_range.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/typical_tableview_config.h"

namespace ui::reference::fragments {
   void activate_parents::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;
      this->model    = new model_type(controls.view);
      this->owner    = &owner;

      ui::set_unsigned_range<float>(controls.edit.delay);
      
      {
         auto* listview = this->controls.view;
         this->model = new model_type(listview);
         listview->setModel(this->model);
         ui::typical_tableview_config(listview);
         ui::set_tableview_column_flex(listview, [this](DKHeaderView& header, const QFontMetrics& metrics) {
            for (int col : std::array{
               model_type::Column::RefName,
               model_type::Column::Delay,
            }) {
               auto title = this->model->headerData(col, header.orientation(), Qt::DisplayRole).toString();
               header.setColumnFlex(col, 1, 1, metrics.horizontalAdvance(title) * 1.5F + 4);
            }
            header.setColumnFlex(model_type::Column::RefFormID, 0, 0, 4);
         });

         QObject::connect(listview->selectionModel(), &QItemSelectionModel::selectionChanged, &owner, cobb__bound_this_fn(_on_selection_changed));

         QObject::connect(this->controls.buttons.add,    &QPushButton::clicked, &owner, cobb__bound_this_fn(_try_add_link));
         QObject::connect(this->controls.buttons.remove, &QPushButton::clicked, &owner, cobb__bound_this_fn(_remove_selected_link));
         
         auto on_changed = cobb__bound_this_fn(_update_selected_link);
         QObject::connect(this->controls.edit.ref,   &DKCompactObjectReferencePicker::refChanged,      &owner, on_changed);
         QObject::connect(this->controls.edit.delay, qOverload<double>(&QDoubleSpinBox::valueChanged), &owner, on_changed);
      }
      controls.edit.ref->setValidationFunction([this](dovah::form_stub* ref) -> bool {
         if (ref == this->controls.edit.ref->ref())
            return true;
         if (!ref)
            return true;
         return !this->model->containsRef(*ref);
      });
   }
   void activate_parents::load(loaded_form_type& form) {
      this->stub = &form.stub;

      this->model->importData(form);
      if (auto* extra = form.extra_data.get_or_create<extra_data_type>()) {
         if (extra->flags & extra_data_type::flag::parent_activate_only)
            this->controls.flags.parent_activate_only->setChecked(true);
      }
   }
   void activate_parents::save(loaded_form_type& form) {
      this->model->exportData(form);

      bool parent_only = this->controls.flags.parent_activate_only->isChecked();
      if (parent_only) {
         auto* extra = form.extra_data.get_or_create<extra_data_type>();
         extra->flags |= extra_data_type::flag::parent_activate_only;
      } else {
         if (auto* extra = form.extra_data.get<extra_data_type>())
            extra->flags &= ~extra_data_type::flag::parent_activate_only;
      }
   }

   void activate_parents::_on_selection_changed(const QItemSelection& sel) {
      bool no_selection = sel.empty();
      this->controls.edit.groupbox->setDisabled(no_selection);
      this->controls.buttons.remove->setDisabled(no_selection);
      if (no_selection)
         return;
      QModelIndex qmi = sel[0].topLeft();

      const auto blockers = std::array{
         QSignalBlocker(this->controls.edit.delay),
         QSignalBlocker(this->controls.edit.ref),
      };
      this->controls.edit.delay->setValue(qmi.data(model_type::DelayRole).value<double>());
      this->controls.edit.ref->setRef(qmi.data(model_type::FormStubRole).value<dovah::form_stub*>());
   }
   void activate_parents::_try_add_link() {
      auto* dialog = new DKCompactObjectReferencePickerDialog(this->owner);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      {
         auto all_refs = this->model->allRefs();
         dialog->setValidationFunction([all_refs](dovah::form_stub* ref) {
            auto it = std::find(all_refs.begin(), all_refs.end(), ref);
            if (it == all_refs.end())
               return false;
            return true;
         });
      }
      if (dialog->exec() == QDialog::Accepted) {
         auto* ref = dialog->value();
         if (ref) {
            auto qmi = this->model->setRefDelay(*ref, 0);
            if (qmi.isValid()) {
               this->controls.view->selectionModel()->select(
                  {
                     qmi.siblingAtColumn(0),
                     qmi.siblingAtColumn(model->columnCount({}) - 1)
                  },
                  QItemSelectionModel::SelectionFlag::ClearAndSelect
               );
            }
         }
      }
   }
   void activate_parents::_remove_selected_link() {
      auto* sel_model = this->controls.view->selectionModel();
      auto  sel       = sel_model->selectedRows();
      if (sel.empty())
         return;
      this->model->removeRow(sel[0].row());
   }
   void activate_parents::_update_selected_link() {
      auto* sel_model = this->controls.view->selectionModel();
      auto  sel       = sel_model->selectedRows();
      if (sel.empty())
         return;
      auto* ref = this->controls.edit.ref->ref();
      if (!ref)
         return;
      this->model->setRow(sel[0].row(), *ref, this->controls.edit.delay->value());
   }
}