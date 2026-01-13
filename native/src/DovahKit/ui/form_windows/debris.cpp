#include "./debris.h"
#include <array>
#include "ui/utils/bind.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/typical_tableview_config.h"
#include "./debris/DebrisVariantsModel.h"

FormDialogDebris::FormDialogDebris(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   {
      auto* view  = this->ui.view;
      auto* model = this->_models.variants = new DebrisVariantsModel(this);
      view->setModel(model);
      ui::typical_tableview_config(view);
      view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      view->setWordWrap(false);
      ui::set_tableview_column_flex(view, [model](DKHeaderView& header, const QFontMetrics& metrics) {
         header.setColumnFlex(DebrisVariantsModel::Column::Path, 2, 0);
         {
            constexpr const auto column = DebrisVariantsModel::Column::Chance;
            auto label_text = model->headerData(column, Qt::Orientation::Horizontal, Qt::DisplayRole).toString();
            auto label_width = metrics.boundingRect(label_text).width();
            auto value_width = metrics.boundingRect("99999").width();
            header.setColumnFlex(column, 0, 0, std::max(label_width, value_width) * 1.5F + 4);
         }
         {
            constexpr const auto column = DebrisVariantsModel::Column::HasCollision;
            auto label_text = model->headerData(column, Qt::Orientation::Horizontal, Qt::DisplayRole).toString();
            auto label_width = metrics.boundingRect(label_text).width();
            auto value_width = metrics.boundingRect("Yes").width();
            header.setColumnFlex(column, 0, 0, std::max(label_width, value_width) * 1.5F + 4);
         }
      });

      auto* sel_model = view->selectionModel();
      QObject::connect(
         sel_model,
         &QItemSelectionModel::selectionChanged,
         this,
         [this, sel_model, model](const QItemSelection& selected, const QItemSelection& deselected) {
            const auto blockers = std::array{
               QSignalBlocker(this->ui.currentModel),
               QSignalBlocker(this->ui.currentPercent),
               QSignalBlocker(this->ui.currentHasCollision),
            };

            if (selected.empty()) {
               this->ui.buttonRemove->setEnabled(false);
               this->ui.current->setEnabled(false);
               this->ui.currentModel->setValue({});
               this->ui.currentPercent->setValue(0);
               this->ui.currentHasCollision->setChecked(false);
               return;
            }
            auto* item = model->item(selected[0].topLeft().row());

            this->ui.buttonRemove->setEnabled(true);
            this->ui.current->setEnabled(true);
            this->ui.currentModel->setValue(item->path);
            this->ui.currentPercent->setValue(item->chance);
            this->ui.currentHasCollision->setChecked(item->has_collision);
         }
      );
      
      auto _on_change = [this, model, sel_model]() {
         auto rows = sel_model->selectedRows();
         if (rows.empty())
            return;
         auto* item = model->item(rows[0].row());
         if (!item)
            return;

         auto copy = *item;
         copy.path          = this->ui.currentModel->value();
         copy.chance        = this->ui.currentPercent->value();
         copy.has_collision = this->ui.currentHasCollision->isChecked();
         model->overwrite(rows[0].row(), copy);
      };
      QObject::connect(this->ui.currentModel, &DKFormNIFPicker::dataChanged, this, _on_change);
      QObject::connect(this->ui.currentPercent, qOverload<int>(&QSpinBox::valueChanged), this, _on_change);
      QObject::connect(this->ui.currentHasCollision, &QCheckBox::toggled, this, _on_change);
      
      QObject::connect(this->ui.buttonAdd, &QPushButton::clicked, this, [this, model, sel_model]() {
         auto qmi = model->create();
         if (qmi.isValid()) {
            sel_model->select({ qmi, qmi.siblingAtColumn(model->columnCount({}) - 1) }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
         }
      });
      QObject::connect(this->ui.buttonRemove, &QPushButton::clicked, this, [this, model, sel_model]() {
         auto rows = sel_model->selectedRows();
         if (rows.empty())
            return;
         model->deleteItems(rows[0].row(), 1);
      });
   }

   this->load(); // this creates the working copy.
}
void FormDialogDebris::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   {
      const auto& src_list = working.variations;
      std::vector<DebrisVariantsModel::node_type> dst_list;
      dst_list.reserve(src_list.size());
      for (const auto& src : src_list) {
         auto& dst = dst_list.emplace_back();
         dst.path.initializeFrom(src.model);
         dst.chance = src.percentage;
         dst.has_collision = src.flags & loaded_form_type::variation_flag::has_collision;
      }
      this->_models.variants->overwriteAllItems(dst_list);
   }
}
void FormDialogDebris::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   {
      const auto* model = this->_models.variants;
      const auto  count = model->rowCount();

      auto& dst_list = working.variations;
      {
         for (auto& item : dst_list)
            item.model.clear();
         dst_list.clear();
         dst_list.reserve(count);
      }
      for (size_t i = 0; i < count; ++i) {
         const auto* src_item = model->item(i);
         auto&       dst_item = dst_list.emplace_back();
         src_item->path.commitTo(dst_item.model, working);
         dst_item.percentage = src_item->chance;
         if (src_item->has_collision)
            dst_item.flags |= loaded_form_type::variation_flag::has_collision;
         else
            dst_item.flags &= ~loaded_form_type::variation_flag::has_collision;
      }
   }
}