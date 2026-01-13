#include "./weather.h"
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableView>
#include "helpers/bound_mem_fn.h"
#include "widgets/DKFormPicker.h"
#include "widgets/widget-dialogs/DKCompactObjectReferencePickerDialog.h"
#include "editor/form_stub_meta_type.h"
#include "ui/model_utils/ViewEventFilter_RemoveRowOnDelKey.h"
#include "ui/utils/set_custom_context_menu.h"
#include "ui/utils/set_range.h"
#include "ui/utils/typical_tableview_config.h"
#include "../RegionWeatherModel.h"
#include "../RegionsDialog.h"
#include "ui/form_windows/shared/DKFormPickerExcludeListedFormsFilter.h"

namespace ui::region::fragments {
   weather::weather(RegionsDialog& o) : owner(o) {
      this->model = new model_type(&o);

      this->remove_row_on_del = new ui::model_utils::ViewEventFilter_RemoveRowOnDelKey(&o);
   }
   void weather::set_controls(controls&& src) {
      this->ui = std::move(src);
      this->ui.edit.container->setEnabled(false);
      this->ui.edit.weather->setAllowedFormType(dovah::form_type::weather);
      this->ui.edit.chance.form.value->setAllowedFormType(dovah::form_type::global);

      {
         auto* view = this->ui.view;
         view->setModel(this->model);
         view->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
         view->setDragDropOverwriteMode(false);
         view->setDragEnabled(true);
         view->setAcceptDrops(true);
         view->setDropIndicatorShown(true);
         ui::typical_tableview_config(view);
         view->installEventFilter(this->remove_row_on_del);

         QObject::connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, &this->owner, [this](const QItemSelection& sel) {
            if (sel.empty()) {
               this->on_no_item_selected();
            } else {
               this->on_item_selected(sel[0].topLeft());
            }
         });

         // Account for drag-and-drop, and ensure the region is considered "edited" when it happens.
         // Ditto for spontaneous removals e.g. due to deletions of referenced forms.
         {
            auto _changed = cobb__bound_this_fn(on_model_layout_edited);
            QObject::connect(this->model, &QAbstractItemModel::rowsInserted, &this->owner, _changed);
            QObject::connect(this->model, &QAbstractItemModel::rowsRemoved,  &this->owner, _changed);
         }

         QObject::connect(this->ui.buttons.add,    &QPushButton::clicked, &this->owner, cobb__bound_this_fn(try_add_weather));
         QObject::connect(this->ui.buttons.remove, &QPushButton::clicked, &this->owner, cobb__bound_this_fn(try_remove_item));
         #pragma region Context menu
         {
            auto& menu = this->view_context.menu;
            ui::set_custom_context_menu(*view, menu);
            {
               auto*& action = this->view_context.actions.insert;
               action = new QAction(QCoreApplication::translate("REGN dialog, objects tab", "Add another weather..."), &menu);
               QObject::connect(action, &QAction::triggered, &this->owner, cobb__bound_this_fn(try_add_weather));
               menu.addAction(action);
            }
            {
               auto*& action = this->view_context.actions.remove;
               action = new QAction(QCoreApplication::translate("REGN dialog, objects tab", "Remove"), &menu);
               QObject::connect(action, &QAction::triggered, &this->owner, cobb__bound_this_fn(try_remove_item));
               menu.addAction(action);
            }
            QObject::connect(&menu, &QMenu::aboutToShow, &this->owner, [this]() {
               bool has_selection = !this->ui.view->selectionModel()->selection().empty();
               this->view_context.actions.remove->setEnabled(has_selection);
            });
         }
         #pragma endregion
      }

      QObject::connect(this->ui.header.enable,  &QCheckBox::toggled, &this->owner, cobb__bound_this_fn(on_header_edited));
      QObject::connect(this->ui.header.override, &QCheckBox::toggled, &this->owner, cobb__bound_this_fn(on_header_edited));
      QObject::connect(this->ui.header.priority, qOverload<int>(&QSpinBox::valueChanged), &this->owner, cobb__bound_this_fn(on_header_edited));

      QObject::connect(this->ui.edit.weather,               &DKFormPicker::formChanged, &this->owner, cobb__bound_this_fn(on_item_edited));
      QObject::connect(this->ui.edit.chance.constant.radio, &QRadioButton::toggled, &this->owner, cobb__bound_this_fn(on_item_edited));
      QObject::connect(this->ui.edit.chance.constant.value, qOverload<int>(&QSpinBox::valueChanged), &this->owner, cobb__bound_this_fn(on_item_edited));
      QObject::connect(this->ui.edit.chance.form.radio,     &QRadioButton::toggled, &this->owner, cobb__bound_this_fn(on_item_edited));
      QObject::connect(this->ui.edit.chance.form.value,     &DKFormPicker::formChanged, &this->owner, cobb__bound_this_fn(on_item_edited));
   }
   void weather::reload() {
      auto& data     = this->owner.region_data({});
      auto& opt_coll = data.generable_content.weather;
      this->ui.header.enable->setEnabled(data.stub != nullptr);
      if (!data.stub || !opt_coll.has_value()) {
         this->ui.header.enable->setChecked(false);
         this->ui.header.override->setChecked(false);
         this->ui.header.override->setEnabled(false);
         this->ui.header.priority->setValue(0);
         this->ui.header.priority->setEnabled(false);
         this->model->clear();
         this->ui.view->setEnabled(false);
         this->ui.buttons.remove->setEnabled(false);
         return;
      }
      auto& src = opt_coll.value();
      this->ui.header.enable->setChecked(true);
      this->ui.header.override->setChecked(src.override);
      this->ui.header.override->setEnabled(true);
      this->ui.header.priority->setValue(src.priority);
      this->ui.header.priority->setEnabled(true);
      this->ui.view->setEnabled(true);
      this->model->importData(data);
   }
   void weather::commit() {
      auto& data     = this->owner.region_data({});
      auto& opt_coll = data.generable_content.weather;
      if (this->ui.header.enable->isChecked()) {
         opt_coll.emplace();
         auto& dst = opt_coll.value();
         dst.override = this->ui.header.override->isChecked();
         dst.priority = this->ui.header.priority->value();
         this->model->exportData(data);
      } else {
         opt_coll.reset();
      }
   }

   void weather::on_item_selected(const QModelIndex& qmi) {
      if (!qmi.isValid()) {
         this->on_no_item_selected();
         return;
      }

      this->ui.edit.container->setEnabled(true);
      this->ui.buttons.remove->setEnabled(true);

      const auto blockers = std::array{
         QSignalBlocker(this->ui.edit.weather),
         QSignalBlocker(this->ui.edit.chance.constant.radio),
         QSignalBlocker(this->ui.edit.chance.constant.value),
         QSignalBlocker(this->ui.edit.chance.form.radio),
         QSignalBlocker(this->ui.edit.chance.form.value),
      };

      auto* global = qmi.data(model_type::GlobalStubRole).value<dovah::form_stub*>();
      this->ui.edit.weather->setFormStub(qmi.data(model_type::WeatherStubRole).value<dovah::form_stub*>());
      this->ui.edit.chance.form.value->setFormStub(global);
      this->ui.edit.chance.constant.value->setValue(qmi.siblingAtColumn(model_type::Column::Chance).data(Qt::EditRole).value<int>());

      bool uses_global = global == nullptr;
      this->ui.edit.chance.constant.radio->setChecked(!uses_global);
      this->ui.edit.chance.constant.value->setEnabled(!uses_global);
      this->ui.edit.chance.form.radio->setChecked(uses_global);
      this->ui.edit.chance.form.value->setEnabled(uses_global);
   }
   void weather::on_no_item_selected() {
      this->ui.edit.container->setEnabled(false);
      this->ui.buttons.remove->setEnabled(false);
   }
   void weather::on_header_edited() {
      bool enabled = this->ui.header.enable->isChecked();
      this->ui.header.override->setEnabled(enabled);
      this->ui.header.priority->setEnabled(enabled);
      this->ui.edit.container->setEnabled(enabled);
      this->ui.view->setEnabled(enabled);
      this->owner.on_region_modified({});
   }
   void weather::on_item_edited() {
      auto sel = this->ui.view->selectionModel()->selection();
      if (sel.empty())
         return;
      QModelIndex qmi = sel[0].topLeft();

      dovah::form_stub* weather = this->ui.edit.weather->formStub();
      dovah::form_stub* global  = nullptr;
      if (this->ui.edit.chance.form.radio->isChecked()) {
         global = this->ui.edit.chance.form.value->formStub();
      }
      this->model->setData(qmi, QVariant::fromValue(weather), model_type::WeatherStubRole);
      this->model->setData(qmi, QVariant::fromValue(global),  model_type::GlobalStubRole);
      this->model->setData(qmi.siblingAtColumn(model_type::Column::Chance), this->ui.edit.chance.constant.value->value(), Qt::EditRole);

      this->owner.on_region_modified({});
   }
   void weather::on_model_layout_edited() {
      this->owner.on_region_modified({});
   }

   void weather::try_add_weather() {
      auto* dialog = new DKCompactObjectReferencePickerDialog(&this->owner);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      {
         auto all_refs = this->model->allWeathers();
         dialog->setValidationFunction([all_refs](dovah::form_stub* ref) {
            auto it = std::find(all_refs.begin(), all_refs.end(), ref);
            if (it == all_refs.end())
               return false;
            return true;
         });
      }
      if (dialog->exec() != QDialog::Accepted)
         return;
      dovah::form_stub* weather = dialog->value();
      if (!weather)
         return;
      auto qmi = this->model->addWeather(*weather, 0, nullptr);
      if (!qmi.isValid())
         return;
      this->ui.view->selectionModel()->select(
         {
            qmi.siblingAtColumn(0),
            qmi.siblingAtColumn(model->columnCount({}) - 1)
         },
         QItemSelectionModel::SelectionFlag::ClearAndSelect
      );
   }
   void weather::try_remove_item() {
      auto sel = this->ui.view->selectionModel()->selection();
      if (sel.empty())
         return;
      QModelIndex qmi = sel[0].topLeft();
      this->model->removeRows(qmi.row(), 1, qmi.parent());
   }
}