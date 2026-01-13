#include "./lens_flare.h"
#include "ui/utils/bind.h"
#include "ui/utils/typical_tableview_config.h"
#include "./lens_flare/FormSubdialogLensFlareSprite.h"
#include "./lens_flare/LensFlareSpritesModel.h"

FormDialogLensFlare::FormDialogLensFlare(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   {
      auto* view  = this->ui.sprites;
      auto* model = this->_models.sprites = new LensFlareSpritesModel(this);
      view->setModel(model);
      ui::typical_tableview_config(view);
      view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      view->setWordWrap(false);
      
      auto* sel_model = view->selectionModel();
      QObject::connect(
         sel_model,
         &QItemSelectionModel::selectionChanged,
         this,
         [this, sel_model, model](const QItemSelection& selected, const QItemSelection& deselected) {
            bool has_sel = !selected.isEmpty();
            this->ui.buttonSpriteEdit->setEnabled(has_sel);
            this->ui.buttonSpriteMoveUp->setEnabled(has_sel);
            this->ui.buttonSpriteMoveDown->setEnabled(has_sel);
            this->ui.buttonSpriteDelete->setEnabled(has_sel);
         }
      );
      this->ui.buttonSpriteEdit->setEnabled(false);
      this->ui.buttonSpriteMoveUp->setEnabled(false);
      this->ui.buttonSpriteMoveDown->setEnabled(false);
      this->ui.buttonSpriteDelete->setEnabled(false);

      #pragma region Buttons
         QObject::connect(this->ui.buttonSpriteNew, &QPushButton::clicked, this, [this, sel_model, model]() {
            auto qmi = model->create();
            if (qmi.isValid()) {
               auto tl = qmi.siblingAtColumn(0);
               auto br = qmi.siblingAtColumn(model->columnCount({}) - 1);
               sel_model->select({ tl, br }, QItemSelectionModel::ClearAndSelect);
            }
         });
         QObject::connect(this->ui.buttonSpriteEdit, &QPushButton::clicked, this, [this, sel_model, model]() {
            auto rows = sel_model->selectedRows();
            if (rows.empty())
               return;
            auto  row  = rows[0].row();
            auto* item = model->item(row);
            if (!item)
               return;

            FormSubdialogLensFlareSprite modal;
            modal.setValue(*item);
            if (modal.exec() == QDialog::Accepted) {
               model->overwrite(row, modal.value());
            }
         });
         QObject::connect(this->ui.buttonSpriteMoveUp, &QPushButton::clicked, this, [this, sel_model, model]() {
            auto rows = sel_model->selectedRows();
            if (rows.empty())
               return;
            auto row = rows[0].row();
            model->moveItem(rows[0], -1);
         });
         QObject::connect(this->ui.buttonSpriteMoveDown, &QPushButton::clicked, this, [this, sel_model, model]() {
            auto rows = sel_model->selectedRows();
            if (rows.empty())
               return;
            auto row = rows[0].row();
            model->moveItem(rows[0], 1);
         });
         QObject::connect(this->ui.buttonSpriteDelete, &QPushButton::clicked, this, [this, sel_model, model]() {
            auto rows = sel_model->selectedRows();
            if (rows.empty())
               return;
            auto row = rows[0].row();
            model->deleteItems(row, 1);
         });
      #pragma endregion
   }

   this->load(); // this creates the working copy.
}
void FormDialogLensFlare::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.colorInfluence, working.color_influence);
   ui::bind(this->ui.fadeDistRadScale, working.fade_distance_radius_scale);

   {
      auto* model = this->_models.sprites;
      model->overwriteAllItems(working.sprites);
   }
}
void FormDialogLensFlare::_save_impl() {
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
      auto* model = this->_models.sprites;
      auto  count = model->rowCount();
      auto& dst   = working.sprites;
      dst.clear();
      dst.reserve(count);
      for (size_t i = 0; i < count; ++i) {
         const auto* item = model->item(i);
         if (item) {
            dst.emplace_back() = *item;
         }
      }
   }
}