#include "./audio.h"
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableView>
#include "helpers/bound_mem_fn.h"
#include "widgets/DKFormPicker.h"
#include "widgets/DKFormPickerDialog.h"
#include "editor/form_stub_meta_type.h"
#include "ui/model_utils/ViewEventFilter_RemoveRowOnDelKey.h"
#include "ui/utils/enable_inbound_drag_and_drop_insertions.h"
#include "ui/utils/set_custom_context_menu.h"
#include "ui/utils/set_range.h"
#include "ui/utils/size_tableview_columns.h"
#include "ui/utils/typical_tableview_config.h"
#include "../RegionSoundsModel.h"
#include "../RegionsDialog.h"
#include "ui/form_windows/shared/DKFormPickerExcludeListedFormsFilter.h"

namespace ui::region::fragments {
   audio::audio(RegionsDialog& o) : owner(o) {
      this->model = new model_type(&o);

      this->remove_row_on_del = new ui::model_utils::ViewEventFilter_RemoveRowOnDelKey(&o);
   }
   void audio::set_controls(controls&& src) {
      this->ui = std::move(src);
      this->ui.music->setAllowedFormType(dovah::form_type::music_type);
      this->ui.music->setEnabled(false);
      this->ui.edit.container->setEnabled(false);
      this->ui.edit.sound->setAllowedFormType(dovah::form_type::sound_descriptor);

      {
         auto* view = this->ui.view;
         view->setModel(this->model);
         view->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
         view->setDragDropOverwriteMode(false);
         view->setDragEnabled(true);
         view->setAcceptDrops(true);
         view->setDropIndicatorShown(true);
         ui::typical_tableview_config(view);
         ui::enable_inbound_drag_and_drop_insertions(view);
         view->installEventFilter(this->remove_row_on_del);
         ui::size_tableview_columns<
            std::array{
               ui::tableview_column_spec{ .grow = 3, .shrink = 1 },
               ui::tableview_column_spec{ .grow = 0, .shrink = 0 },
               ui::tableview_column_spec{ .grow = 0, .shrink = 0 },
               ui::tableview_column_spec{ .grow = 0, .shrink = 0 },
               ui::tableview_column_spec{ .grow = 0, .shrink = 0 },
               ui::tableview_column_spec{ .grow = 0, .shrink = 0 },
            }
         >(view);

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

         QObject::connect(this->ui.buttons.add,    &QPushButton::clicked, &this->owner, cobb__bound_this_fn(try_add_sound));
         QObject::connect(this->ui.buttons.remove, &QPushButton::clicked, &this->owner, cobb__bound_this_fn(try_remove_item));
         #pragma region Context menu
         {
            auto& menu = this->view_context.menu;
            ui::set_custom_context_menu(*view, menu);
            {
               auto*& action = this->view_context.actions.insert;
               action = new QAction(QCoreApplication::translate("REGN dialog, audio tab", "Add another sound..."), &menu);
               QObject::connect(action, &QAction::triggered, &this->owner, cobb__bound_this_fn(try_add_sound));
               menu.addAction(action);
            }
            {
               auto*& action = this->view_context.actions.remove;
               action = new QAction(QCoreApplication::translate("REGN dialog, audio tab", "Remove"), &menu);
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

      QObject::connect(this->ui.edit.sound,            &DKFormPicker::formChanged, &this->owner, cobb__bound_this_fn(on_item_edited));
      QObject::connect(this->ui.edit.chance,           qOverload<double>(&QDoubleSpinBox::valueChanged), &this->owner, cobb__bound_this_fn(on_item_edited));
      QObject::connect(this->ui.edit.weather.pleasant, &QCheckBox::toggled, &this->owner, cobb__bound_this_fn(on_item_edited));
      QObject::connect(this->ui.edit.weather.cloudy,   &QCheckBox::toggled, &this->owner, cobb__bound_this_fn(on_item_edited));
      QObject::connect(this->ui.edit.weather.rainy,    &QCheckBox::toggled, &this->owner, cobb__bound_this_fn(on_item_edited));
      QObject::connect(this->ui.edit.weather.snowy,    &QCheckBox::toggled, &this->owner, cobb__bound_this_fn(on_item_edited));
   }
   void audio::reload() {
      auto& data     = this->owner.region_data({});
      auto& opt_coll = data.generable_content.audio;
      this->ui.header.enable->setEnabled(data.stub != nullptr);
      if (!data.stub || !opt_coll.has_value()) {
         this->ui.header.enable->setChecked(false);
         this->ui.header.override->setChecked(false);
         this->ui.header.override->setEnabled(false);
         this->ui.header.priority->setValue(0);
         this->ui.header.priority->setEnabled(false);
         this->model->clear();
         this->ui.music->setEnabled(false);
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
      this->ui.music->setEnabled(true);
      this->ui.view->setEnabled(true);
      this->model->importData(data);
      this->on_no_item_selected();
   }
   void audio::commit() {
      auto& data     = this->owner.region_data({});
      auto& opt_coll = data.generable_content.audio;
      if (this->ui.header.enable->isChecked()) {
         opt_coll.emplace();
         auto& dst = opt_coll.value();
         dst.override = this->ui.header.override->isChecked();
         dst.priority = this->ui.header.priority->value();
         dst.music_type = this->ui.music->formStub();
         this->model->exportData(data);
      } else {
         opt_coll.reset();
      }
   }

   void audio::on_item_selected(const QModelIndex& qmi) {
      if (!qmi.isValid()) {
         this->on_no_item_selected();
         return;
      }

      this->ui.edit.container->setEnabled(true);
      this->ui.buttons.remove->setEnabled(true);

      const auto blockers = std::array{
         QSignalBlocker(this->ui.edit.sound),
         QSignalBlocker(this->ui.edit.chance),
         QSignalBlocker(this->ui.edit.weather.pleasant),
         QSignalBlocker(this->ui.edit.weather.cloudy),
         QSignalBlocker(this->ui.edit.weather.rainy),
         QSignalBlocker(this->ui.edit.weather.snowy),
      };

      auto* sound = qmi.data(model_type::FormStubRole).value<dovah::form_stub*>();
      this->ui.edit.sound->setFormStub(sound);
      this->ui.edit.chance->setValue(qmi.siblingAtColumn(model_type::Column::Chance).data(Qt::EditRole).value<int>());
      this->ui.edit.weather.pleasant->setChecked(qmi.siblingAtColumn(model_type::Column::WeatherIsPleasant).data(Qt::EditRole).toBool());
      this->ui.edit.weather.cloudy->setChecked(qmi.siblingAtColumn(model_type::Column::WeatherIsCloudy).data(Qt::EditRole).toBool());
      this->ui.edit.weather.rainy->setChecked(qmi.siblingAtColumn(model_type::Column::WeatherIsRainy).data(Qt::EditRole).toBool());
      this->ui.edit.weather.snowy->setChecked(qmi.siblingAtColumn(model_type::Column::WeatherIsSnowy).data(Qt::EditRole).toBool());
   }
   void audio::on_no_item_selected() {
      this->ui.edit.container->setEnabled(false);
      this->ui.buttons.remove->setEnabled(false);
   }
   void audio::on_header_edited() {
      bool enabled = this->ui.header.enable->isChecked();
      this->ui.header.override->setEnabled(enabled);
      this->ui.header.priority->setEnabled(enabled);
      this->ui.view->setEnabled(enabled);
      {
         auto* sel_model = this->ui.view->selectionModel();
         auto  rows      = sel_model->selectedRows();
         if (rows.empty())
            this->on_no_item_selected();
         else
            this->on_item_selected(rows[0]);
      }
      this->owner.on_region_modified({});
   }
   void audio::on_item_edited() {
      auto sel = this->ui.view->selectionModel()->selection();
      if (sel.empty())
         return;
      QModelIndex qmi = sel[0].topLeft();

      dovah::form_stub* sound = this->ui.edit.sound->formStub();
      this->model->setData(qmi, QVariant::fromValue(sound), model_type::FormStubRole);
      this->model->setData(qmi.siblingAtColumn(model_type::Column::Chance), this->ui.edit.chance->value(), Qt::EditRole);
      this->model->setData(qmi.siblingAtColumn(model_type::Column::WeatherIsPleasant), this->ui.edit.weather.pleasant->isChecked(), Qt::EditRole);
      this->model->setData(qmi.siblingAtColumn(model_type::Column::WeatherIsCloudy),   this->ui.edit.weather.cloudy->isChecked(), Qt::EditRole);
      this->model->setData(qmi.siblingAtColumn(model_type::Column::WeatherIsRainy),    this->ui.edit.weather.rainy->isChecked(), Qt::EditRole);
      this->model->setData(qmi.siblingAtColumn(model_type::Column::WeatherIsSnowy),    this->ui.edit.weather.snowy->isChecked(), Qt::EditRole);

      this->owner.on_region_modified({});
   }
   void audio::on_model_layout_edited() {
      this->owner.on_region_modified({});
   }

   void audio::try_add_sound() {
      auto* dialog = new DKFormPickerDialog(&this->owner);
      dialog->setAllowedFormType(dovah::form_type::sound_descriptor);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      if (dialog->exec() != QDialog::Accepted)
         return;
      dovah::form_stub* form = dialog->formStub();
      if (!form)
         return;
      size_t row = this->model->rowCount({});
      this->model->insertRow(row, {});
      auto qmi = this->model->index(row, 0, {});
      if (!qmi.isValid())
         return;
      this->model->setData(qmi, QVariant::fromValue(form), model_type::FormStubRole);
      this->ui.view->selectionModel()->select(
         {
            qmi.siblingAtColumn(0),
            qmi.siblingAtColumn(model->columnCount({}) - 1)
         },
         QItemSelectionModel::SelectionFlag::ClearAndSelect
      );
   }
   void audio::try_remove_item() {
      auto sel = this->ui.view->selectionModel()->selection();
      if (sel.empty())
         return;
      QModelIndex qmi = sel[0].topLeft();
      this->model->removeRows(qmi.row(), 1, qmi.parent());
   }
}