#include "./water_reflectee.h"
#include <QGroupBox>
#include <QPushButton>
#include <QTableView>
#include <QVariant>
#include "helpers/bound_mem_fn.h"
#include "widgets/widget-dialogs/DKCompactObjectReferencePickerDialog.h"
#include "widgets/DKHeaderView.h"
#include "dovah/forms/ObjectReference.h"
#include "editor/form_stub_meta_type.h"
#include "../ObjectReferenceReflectingWaterModel.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/typical_tableview_config.h"

namespace ui::reference::fragments {
   void water_reflectee::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;
      this->model    = new model_type(controls.view);
      this->owner    = &owner;
      
      auto* listview = this->controls.view;
      this->model = new model_type(listview);
      listview->setModel(this->model);
      ui::typical_tableview_config(listview);
      ui::set_tableview_column_flex(controls.view, [this](DKHeaderView& header, const QFontMetrics& metrics) {
         for (int col : std::array{
            model_type::Column::RefName,
            model_type::Column::Type,
         }) {
            auto title = this->model->headerData(col, header.orientation(), Qt::DisplayRole).toString();
            header.setColumnFlex(col, 1, 1, metrics.horizontalAdvance(title) * 1.5F + 4);
         }
         header.setColumnFlex(model_type::Column::RefFormID, 0, 0, 4);
      });
      {
         auto* delegate = new model_type::ReflectionTypeItemDelegate(this->model);
         listview->setItemDelegateForColumn(model_type::Column::Type, delegate);
      }

      QObject::connect(listview->selectionModel(), &QItemSelectionModel::selectionChanged, &owner, cobb__bound_this_fn(_on_selection_changed));

      QObject::connect(this->controls.buttons.add,    &QPushButton::clicked, &owner, cobb__bound_this_fn(_try_add_water));
      QObject::connect(this->controls.buttons.remove, &QPushButton::clicked, &owner, cobb__bound_this_fn(_try_remove_water));
   }
   void water_reflectee::load(loaded_form_type& form) {
      this->stub = &form.stub;

      bool is_light = false;
      if (auto* base_form = form.base_form.get_form_stub())
         is_light = base_form->form_type == dovah::form_type::light;

      this->controls.groupbox->setEnabled(true);
      if (is_light) {
         this->model->import_data(model_type::ExtraDataType::LitWater, form);
      } else {
         this->model->import_data(model_type::ExtraDataType::ReflectorRefs, form);
      }
   }
   void water_reflectee::save(loaded_form_type& form) {
      this->model->export_data(form);
   }

   void water_reflectee::_on_selection_changed(const QItemSelection& sel) {
      bool no_selection = sel.empty();
      this->controls.buttons.add->setDisabled(no_selection);
      this->controls.buttons.remove->setDisabled(no_selection);
   }
   void water_reflectee::_try_add_water() {
      auto* dialog = new DKCompactObjectReferencePickerDialog(this->owner);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      dialog->setValidationFunction([this](dovah::form_stub* ref) -> bool {
         if (!ref)
            return true;
         return this->model->is_valid_reflector(*ref);
      });
      if (dialog->exec() == QDialog::Accepted) {
         auto* ref = dialog->value();
         if (ref)
            this->model->add_reflector(*ref);
      }
   }
   void water_reflectee::_try_remove_water() {
      auto* sel_model = this->controls.view->selectionModel();
      auto  sel       = sel_model->selectedRows();
      if (sel.empty())
         return;
      auto  qmi  = sel[0];
      auto* refr = qmi.data(model_type::FormStubRole).value<dovah::form_stub*>();
      if (!refr)
         return;
      this->model->remove_reflector(*refr);
   }
}