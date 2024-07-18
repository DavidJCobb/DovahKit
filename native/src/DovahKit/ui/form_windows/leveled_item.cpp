#include "./leveled_item.h"
#include <QKeyEvent>
#include "dovah/core.h"
#include "dovah/utils/leveled_list_preview.h"
#include "ui/models/forms/LeveledListModel.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "widgets/DKHeaderView.h"
#include "./leveled_lists/LeveledListPreviewResultsWindow.h"

namespace {
   // Multiply the health value by this when displaying it.
   constexpr const float health_display_mult = 100;
}

FormDialogLeveledItem::FormDialogLeveledItem(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.chanceNonePercentage->setRange(0, 100);
   this->ui.chanceNoneGlobal->setAllowedFormType(dovah::form_type::global);

   ui::set_range<uint16_t>(this->ui.entryCount);
   ui::set_range<uint16_t>(this->ui.entryLevel);
   ui::set_unsigned_range<float>(this->ui.entryHealth);
   ui::set_range<uint16_t>(this->ui.entryOwnerRank);

   this->ui.entryOwnerActorBase->setAllowedFormType(dovah::form_type::actor_base);
   this->ui.entryOwnerFaction->setAllowedFormType(dovah::form_type::faction);
   this->ui.entryOwnerGlobal->setAllowedFormType(dovah::form_type::global);

   // Handle mutually-exclusive sets of flags.
   {
      auto handler = [this]() {
         bool allow_use_all = true;
         if (this->ui.flagCumulativeLevels->isChecked())
            allow_use_all = false;
         else if (this->ui.flagRecalcPerCount->isChecked())
            allow_use_all = false;

         this->ui.flagUseAll->setEnabled(allow_use_all);
      };
      QObject::connect(this->ui.flagCumulativeLevels, &QCheckBox::toggled, this, handler);
      QObject::connect(this->ui.flagRecalcPerCount,   &QCheckBox::toggled, this, handler);
   }
   QObject::connect(this->ui.flagUseAll, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.flagCumulativeLevels->setEnabled(!checked);
      this->ui.flagRecalcPerCount->setEnabled(!checked);
   });

   this->_model = new LeveledListModel(this);
   {
      auto* view      = this->ui.view;
      auto* model     = this->_model;
      view->setModel(model);
      auto* sel_model = view->selectionModel();

      {
         auto* header = new DKHeaderView(Qt::Horizontal, view);
         header->setFlexResizeEnabled(true);
         view->setHorizontalHeader(header);
         //
         auto metrics = QFontMetrics(view->font());
         header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
         header->setColumnFlex(LeveledListModel::Column::Level,  0, 0, metrics.boundingRect("99").width() * 1.5F + 4);
         header->setColumnFlex(LeveledListModel::Column::Count,  0, 0, metrics.boundingRect("9999").width() * 1.5F + 4);
         header->setColumnFlex(LeveledListModel::Column::Form,   2, 0);
         header->setColumnFlex(LeveledListModel::Column::Health, 0, 0, metrics.boundingRect("100%").width() * 1.5F + 4);
         header->setColumnFlex(LeveledListModel::Column::Owner,  1, 0);
         header->setSectionResizeMode(LeveledListModel::Column::Level,  QHeaderView::Interactive);
         header->setSectionResizeMode(LeveledListModel::Column::Count,  QHeaderView::Interactive);
         header->setSectionResizeMode(LeveledListModel::Column::Form,   QHeaderView::Interactive);
         header->setSectionResizeMode(LeveledListModel::Column::Health, QHeaderView::Interactive);
         header->setSectionResizeMode(LeveledListModel::Column::Owner,  QHeaderView::Interactive);
         header->setStretchLastSection(false);
      }
      if (auto* header = view->verticalHeader()) {
         header->setSectionResizeMode(QHeaderView::ResizeToContents);
         header->setVisible(false);
      }
      view->setSelectionBehavior(QAbstractItemView::SelectRows);
      view->setSelectionMode(QAbstractItemView::SingleSelection);
      view->setCornerButtonEnabled(false);

      QObject::connect(sel_model, &QItemSelectionModel::currentChanged, this, [this, model](const QModelIndex& current, const QModelIndex& previous) {
         const auto blockers = std::array{
            QSignalBlocker(this->ui.entryForm),
            QSignalBlocker(this->ui.entryLevel),
            QSignalBlocker(this->ui.entryCount),
            QSignalBlocker(this->ui.entryHealth),
            QSignalBlocker(this->ui.entryOwnerTypeActorBase),
            QSignalBlocker(this->ui.entryOwnerTypeFaction),
            QSignalBlocker(this->ui.entryOwnerActorBase),
            QSignalBlocker(this->ui.entryOwnerFaction),
            QSignalBlocker(this->ui.entryOwnerGlobal),
            QSignalBlocker(this->ui.entryOwnerRank),
         };

         const auto* data = model->data(current.row());
         bool enable = data != nullptr;

         this->ui.entryForm->setEnabled(enable);
         this->ui.entryLevel->setEnabled(enable);
         this->ui.entryCount->setEnabled(enable);
         this->ui.entryHealth->setEnabled(enable);
         this->ui.entryOwnerGroupbox->setEnabled(enable);
         if (!data) {
            return;
         }

         if (data->form) {
            bool has_health = false;
            bool has_owner  = true;
            switch (data->form->form_type) {
               case dovah::form_type::leveled_item:
               case dovah::form_type::leveled_character:
               case dovah::form_type::leveled_spell:
                  has_owner = false;
                  break;
               case dovah::form_type::armor:
               case dovah::form_type::weapon:
                  has_health = true;
                  break;
            }
            this->ui.entryHealth->setEnabled(has_health);
            this->ui.entryOwnerGroupbox->setEnabled(has_owner);
         } else {
            this->ui.entryHealth->setEnabled(false);
            this->ui.entryOwnerGroupbox->setEnabled(false);
         }

         this->ui.entryForm->setFormStub(data->form);
         this->ui.entryLevel->setValue(data->level);
         this->ui.entryCount->setValue(data->count);
         this->ui.entryHealth->setValue(data->health * health_display_mult);
         {
            auto  type = dovah::form_type::none;
            auto* stub = data->ownership.owner;
            if (stub)
               type = stub->form_type;
            //
            switch (type) {
               case dovah::form_type::actor_base:
                  this->ui.entryOwnerTypeActorBase->setChecked(true);
                  this->ui.entryOwnerActorBase->setFormStub(stub);
                  break;
               case dovah::form_type::faction:
                  this->ui.entryOwnerTypeFaction->setChecked(true);
                  this->ui.entryOwnerFaction->setFormStub(stub);
                  break;
               default:
                  this->ui.entryOwnerTypeActorBase->setChecked(true);
                  this->ui.entryOwnerActorBase->setFormStub(nullptr);
                  break;
            }
         }
         this->ui.entryOwnerGlobal->setFormStub(data->ownership.global);
         this->ui.entryOwnerRank->setValue(data->ownership.rank);
      });
      QObject::connect(this->ui.entryForm, &DKFormPicker::formChanged, this, &FormDialogLeveledItem::_overwrite_selected_leveled_object);
      QObject::connect(this->ui.entryLevel, QOverload<int>::of(&QSpinBox::valueChanged), this, &FormDialogLeveledItem::_overwrite_selected_leveled_object);
      QObject::connect(this->ui.entryCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &FormDialogLeveledItem::_overwrite_selected_leveled_object);
      QObject::connect(this->ui.entryHealth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormDialogLeveledItem::_overwrite_selected_leveled_object);
      QObject::connect(this->ui.entryOwnerTypeActorBase, &QRadioButton::toggled, this, &FormDialogLeveledItem::_overwrite_selected_leveled_object);
      QObject::connect(this->ui.entryOwnerTypeFaction, &QRadioButton::toggled, this, &FormDialogLeveledItem::_overwrite_selected_leveled_object);
      QObject::connect(this->ui.entryOwnerActorBase, &DKFormPicker::formChanged, this, &FormDialogLeveledItem::_overwrite_selected_leveled_object);
      QObject::connect(this->ui.entryOwnerFaction, &DKFormPicker::formChanged, this, &FormDialogLeveledItem::_overwrite_selected_leveled_object);
      QObject::connect(this->ui.entryOwnerGlobal, &DKFormPicker::formChanged, this, &FormDialogLeveledItem::_overwrite_selected_leveled_object);
      QObject::connect(this->ui.entryOwnerRank, QOverload<int>::of(&QSpinBox::valueChanged), this, &FormDialogLeveledItem::_overwrite_selected_leveled_object);

      // force UI enable state updates:
      sel_model->currentChanged({}, {});

      {
         auto* menu = this->_view_context = new QMenu(this);

         auto* action_new = new QAction(tr("Add entry"), menu);
         QObject::connect(action_new, &QAction::triggered, this, [this]() {
            auto at = this->_model->rowCount();
            if (this->_model->insertRows(at, 1)) {
               auto  qmi       = this->_model->index(at, 0, {});
               auto* sel_model = this->ui.view->selectionModel();
               //sel_model->setCurrentIndex(qmi, QItemSelectionModel::SelectionFlag::ClearAndSelect);

               auto tl = qmi.siblingAtColumn(0);
               auto br = qmi.siblingAtColumn(this->_model->columnCount() - 1);
               sel_model->select(
                  QItemSelection{ tl, br },
                  QItemSelectionModel::SelectionFlag::ClearAndSelect
               );
            }
         });
         menu->addAction(action_new);

         auto* action_delete = new QAction(tr("Remove entry"), menu);
         QObject::connect(action_delete, &QAction::triggered, this, [this]() {
            auto* sel_model = this->ui.view->selectionModel();
            auto  rows      = sel_model->selectedRows();
            if (rows.empty())
               return;
            this->_model->removeRow(rows[0].row());
         });
         menu->addAction(action_delete);

         QObject::connect(menu, &QMenu::aboutToShow, this, [this, action_delete]() {
            auto* sel_model = this->ui.view->selectionModel();
            auto  rows      = sel_model->selectedRows();
            if (rows.empty())
               return;
            action_delete->setVisible(rows[0].isValid());
         });

         view->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
         QObject::connect(view, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
            this->_view_context->exec(this->ui.view->mapToGlobal(pos));
         });
      }

      // hook the Del key
      view->installEventFilter(this);
   }

   QObject::connect(this->ui.entryOwnerTypeActorBase, &QRadioButton::toggled, this, [this](bool checked) {
      this->ui.entryOwnerGlobal->setEnabled(checked);
   });
   QObject::connect(this->ui.entryOwnerTypeFaction, &QRadioButton::toggled, this, [this](bool checked) {
      this->ui.entryOwnerRank->setEnabled(checked);
   });

   ui::set_range<uint16_t>(this->ui.previewCount);
   ui::set_range<uint16_t>(this->ui.previewLevel);
   //
   this->ui.previewCount->setValue(1);
   this->ui.previewLevel->setValue(1);
   //
   QObject::connect(this->ui.buttonPreviewLLCalc, &QPushButton::clicked, this, [this]() {
      using backend_type = typename LeveledListModel::backend_type;

      auto& working = *this->form;

      // The backend has code to generate the results of a leveled list. This means that 
      // we need to actually have our data *in* the backend, though... so commit early 
      // to the working copy. It's fine -- it's just a working copy.
      this->_model->commitTo(working, working.leveled_list_data);

      dovah::leveled_list_preview preview;
      preview.prepare_game_settings(this->formStub()->get_owning_load_order(), working.leveled_list_data);
      preview.input_count  = this->ui.previewCount->value();
      preview.input_level  = this->ui.previewLevel->value();
      preview.player_level = preview.input_level;
      preview.use_special_loot_formula = this->ui.flagSpecialLoot->isChecked();
      auto results = preview.generate(working.leveled_list_data);

      auto* window = new LeveledListPreviewResultsWindow(this);
      window->setContents(results);
      window->show();
   });

   this->load(); // this creates the working copy.
}

void FormDialogLeveledItem::_overwrite_selected_leveled_object() {
   #if _DEBUG
      auto* _widget_what_triggered_this_here_call = sender();
   #endif

   auto* view      = this->ui.view;
   auto* model     = this->_model;
   auto* sel_model = view->selectionModel();

   auto rows = sel_model->selectedRows();
   if (rows.empty())
      return;
   auto row = rows[0].row();
   if (row < 0)
      return;

   auto* src = model->data(row);
   if (!src)
      return;

   LeveledListModel::LeveledObject dst;
   dst.form   = this->ui.entryForm->formStub();
   dst.level  = this->ui.entryLevel->value();
   dst.count  = this->ui.entryCount->value();
   dst.health = this->ui.entryHealth->value() / health_display_mult;
   if (this->ui.entryOwnerTypeActorBase->isChecked()) {
      dst.ownership.owner = this->ui.entryOwnerActorBase->formStub();
   } else {
      dst.ownership.owner = this->ui.entryOwnerFaction->formStub();
   }
   dst.ownership.global = this->ui.entryOwnerGlobal->formStub();
   dst.ownership.rank   = this->ui.entryOwnerRank->value();

   auto qpmi = QPersistentModelIndex(rows[0]);
   model->setData(row, dst);
   //
   // In case the level was changed and the row was moved:
   //
   auto new_qmi = QModelIndex(qpmi);
   auto tl = new_qmi.siblingAtColumn(0);
   auto br = new_qmi.siblingAtColumn(this->_model->columnCount() - 1);
   sel_model->select(
      QItemSelection{ tl, br },
      QItemSelectionModel::SelectionFlag::ClearAndSelect
   );
}

void FormDialogLeveledItem::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   using component_type = decltype(loaded_form_type::leveled_list_data);

   ui::bind(this->ui.editorID, this->editor_id());

   ui::bind(this->ui.flagCumulativeLevels, working.leveled_list_data.flags, component_type::flag::calculate_from_all_levels_below_player);
   ui::bind(this->ui.flagRecalcPerCount,   working.leveled_list_data.flags, component_type::flag::calculate_for_each_item_in_count);
   ui::bind(this->ui.flagUseAll,           working.leveled_list_data.flags, component_type::flag::use_all);
   ui::bind(this->ui.flagSpecialLoot,      working.leveled_list_data.flags, component_type::flag::special_loot);

   ui::bind(this->ui.chanceNoneGlobal,     working.leveled_list_data.chance_none.global, working);
   ui::bind(this->ui.chanceNonePercentage, working.leveled_list_data.chance_none.percentage);

   {
      auto types = working.leveled_list_data.legal_form_types();
      this->ui.entryForm->setAllowedFormTypes(QList<dovah::form_type>(types.begin(), types.end()));
      this->ui.entryForm->setAllowNone(false);
   }
   this->_model->importFrom(working.leveled_list_data);
}
void FormDialogLeveledItem::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& working = *this->form;
   
   this->_model->commitTo(working, working.leveled_list_data);
}

/*virtual*/ bool FormDialogLeveledItem::eventFilter(QObject* watched, QEvent* event) /*override*/ {
   if (watched == this->ui.view) {
      if (event->type() == QEvent::Type::KeyPress) {
         auto* casted = (QKeyEvent*)event;
         if (casted->key() == Qt::Key::Key_Delete) {
            auto* sel_model = this->ui.view->selectionModel();
            auto  rows      = sel_model->selectedRows();
            if (rows.empty())
               return false;
            this->_model->removeRow(rows[0].row());
            return true;
         }
      }
   }
   return false;
}