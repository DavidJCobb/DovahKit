#include "./DKMagicEffectListWidget.h"
#include <QHeaderView>
#include <QVBoxLayout>
#if !defined(QT_PLUGIN)
   #include "./widget-dialogs/DKMagicEffectListItemDialog.h"
   #include "./widget-models/DKMagicEffectListModel.h"
   #include "dovah/forms/Form.h"
   #include "dovah/utils/magic_effect_list_item_cost_calculator.h"
   #include "editor/core.h"
#endif

DKMagicEffectListWidget::DKMagicEffectListWidget(QWidget* parent) : QWidget(parent) {
   auto* layout = new QVBoxLayout(this);
   layout->setMargin(0);
   this->setLayout(layout);

   {
      auto* view = this->_subwidgets.view = new QTableView(this);
      layout->addWidget(view, 1);

      if (auto* vh = view->verticalHeader()) {
         vh->setVisible(false);
         vh->setSectionResizeMode(QHeaderView::ResizeToContents);
      }
      view->setCornerButtonEnabled(false);
      view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      view->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
      view->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
      view->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
      view->setWordWrap(false);
   }
   
   #pragma region Tab order
      this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      this->setFocusProxy(this->_subwidgets.view);
   #pragma endregion

   #if !defined(QT_PLUGIN)
   {
      auto* widget = this->_subwidgets.view;
      this->_model = new DKMagicEffectListModel(this);
      widget->setModel(this->_model);

      {  // Set up new header
         //
         // Have to do this after setting the model, because QHeaderView::setSectionResizeMode 
         // and friends will crash if the section in question doesn't exist yet.
         //
         auto* header = new QHeaderView(Qt::Orientation::Horizontal, widget);
         widget->setHorizontalHeader(header);

         header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
         header->setMinimumSectionSize(2);
         header->setStretchLastSection(true);

         auto metrics = widget->fontMetrics();
         auto _size   = [this, &metrics, header](int col, const char* example_value) {
            QString label = this->_model->headerData(col, Qt::Orientation::Horizontal, Qt::DisplayRole).toString();
            auto    width = std::max(metrics.horizontalAdvance(label), metrics.horizontalAdvance(example_value));
            header->setSectionResizeMode(col, QHeaderView::ResizeMode::Interactive);
            header->resizeSection(col, width);
         };

         _size(DKMagicEffectListModel::Column::Index,       "000");
         _size(DKMagicEffectListModel::Column::Magnitude,   "00000");
         _size(DKMagicEffectListModel::Column::Area,        "000");
         _size(DKMagicEffectListModel::Column::Duration,    "000");
         _size(DKMagicEffectListModel::Column::Cost,        "000");
         _size(DKMagicEffectListModel::Column::MagicSchool, "Restoration000");
         header->setSectionResizeMode(DKMagicEffectListModel::Column::Name, QHeaderView::ResizeMode::Stretch);
      }

      QObject::connect(this->_model, &QAbstractItemModel::dataChanged,  this, &DKMagicEffectListWidget::contentsChanged);
      QObject::connect(this->_model, &QAbstractItemModel::rowsInserted, this, &DKMagicEffectListWidget::contentsChanged);
      QObject::connect(this->_model, &QAbstractItemModel::rowsRemoved,  this, &DKMagicEffectListWidget::contentsChanged);
      QObject::connect(this->_model, &QAbstractItemModel::modelReset,   this, &DKMagicEffectListWidget::contentsChanged);
   }
   QObject::connect(this->_subwidgets.view, &QTableView::doubleClicked, this, [this](const QModelIndex& qmi) {
      auto start = qmi.siblingAtColumn(0);
      auto end   = qmi.siblingAtColumn(DKMagicEffectListModel::Column::_COUNT - 1);

      auto* sm = this->_subwidgets.view->selectionModel();
      sm->select(QItemSelection(start, end), QItemSelectionModel::SelectionFlag::ClearAndSelect);

      this->openEditEffectModal();
   });

   // NOTE: This must be registered after we create our model, so that our model reacts to formModified first.
   QObject::connect(&DovahKitCore::get(), &DovahKitCore::formModified, this, [this]() {
      this->_update_cached_data();
   });

   {
      auto& menu = this->_context_menu.menu;
      {
         auto* action = this->_context_menu.actions.add = new QAction(tr("New"), this);
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, &DKMagicEffectListWidget::_new_effect);
      }
      {
         auto* action = this->_context_menu.actions.edit = new QAction(tr("Edit"), this);
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, &DKMagicEffectListWidget::_edit_effect);
      }
      {
         auto* action = this->_context_menu.actions.remove = new QAction(tr("Delete"), this);
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, &DKMagicEffectListWidget::_delete_effects);
      }

      auto* widget = this->_subwidgets.view;
      auto* sm     = widget->selectionModel();
      QObject::connect(sm, &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection& sel) {
         auto& actions = this->_context_menu.actions;
         if (sel.isEmpty()) {
            actions.edit->setEnabled(false);
            actions.remove->setEnabled(false);
            return;
         }
         actions.edit->setEnabled(sel.size() == 1);
         actions.remove->setEnabled(true);
      });
      QObject::connect(this->_model, &QAbstractItemModel::rowsInserted, this, &DKMagicEffectListWidget::_update_can_add_effect);
      QObject::connect(this->_model, &QAbstractItemModel::rowsRemoved,  this, &DKMagicEffectListWidget::_update_can_add_effect);
   }
   #endif
}

#if !defined(QT_PLUGIN)
   void DKMagicEffectListWidget::setCastingType(const std::optional<dovah::magic_casting_type>& v) {
      if (v == this->_state.casting_type)
         return;
      this->_state.casting_type = v;
      //static_assert(false, "TODO: Show/update warning icons for existing effects in the list that no longer match this casting type");
   }

   void DKMagicEffectListWidget::setDeliveryType(const std::optional<dovah::magic_delivery_type>& v) {
      if (v == this->_state.delivery_type)
         return;
      this->_state.delivery_type = v;
      //static_assert(false, "TODO: Show/update warning icons for existing effects in the list that no longer match this delivery type");
   }

   void DKMagicEffectListWidget::importFrom(dovah::loaded_forms::Form& owner, const dovah::loaded_forms::components::magic_effect_list& target) {
      this->_state.form = &owner;
      this->_model->importFrom(owner, target);
      this->_update_cached_data();
      this->_update_can_add_effect();
   }
   void DKMagicEffectListWidget::exportTo(dovah::loaded_forms::Form& owner, dovah::loaded_forms::components::magic_effect_list& dst) {
      this->_model->commitTo(owner, dst);
   }
#endif

#if !defined(QT_PLUGIN)
   void DKMagicEffectListWidget::openCreateEffectModal() {
      auto* model = this->_model;
      auto* sm    = this->_subwidgets.view->selectionModel();
      if (!this->_state.form || !model || !sm)
         return;
      size_t insert_at = std::numeric_limits<size_t>::max();
      auto   rows      = sm->selectedRows();
      if (!rows.isEmpty())
         insert_at = rows.back().row() + 1;
      {
         DKMagicEffectListModel::Item created;

         auto context = DKMagicEffectListItemDialog::Context{
            .form             = *this->_state.form,
            .casting_type     = this->_state.casting_type,
            .delivery_type    = this->_state.delivery_type,
            .spell_total_cost = this->_state.cached.auto_calculated_cost,
         };
         auto* modal = new DKMagicEffectListItemDialog(context, this);
         modal->setData(created);
         QObject::connect(modal, &QDialog::accepted, this, [this, model, modal, insert_at]() {
            if (!model->insertRows(insert_at, 1))
               return;
            model->setData(insert_at, modal->data());
            this->_update_can_add_effect();
            auto* sm  = this->_subwidgets.view->selectionModel();
            if (!sm)
               return;
            QModelIndex    tl = model->index(insert_at, 0, {});
            QModelIndex    br = model->index(insert_at, model->columnCount() - 1, {});
            QItemSelection range(tl, br);
            sm->select(range, QItemSelectionModel::ClearAndSelect);
         });
         QObject::connect(modal, &QDialog::finished, modal, &QObject::deleteLater);
         modal->setWindowModality(Qt::WindowModality::WindowModal);
         modal->open();
      }
   }
   void DKMagicEffectListWidget::openEditEffectModal() {
      auto* model = this->_model;
      auto* sm    = this->_subwidgets.view->selectionModel();
      if (!this->_state.form || !model || !sm)
         return;
      auto  rows = sm->selectedRows();
      if (rows.size() != 1)
         return;
      auto  row  = rows[0].row();
      auto* item = model->data(row);
      if (!item)
         return;
      {
         auto context = DKMagicEffectListItemDialog::Context{
            .form             = *this->_state.form,
            .casting_type     = this->_state.casting_type,
            .delivery_type    = this->_state.delivery_type,
            .spell_total_cost = this->_state.cached.auto_calculated_cost,
         };
         auto* modal = new DKMagicEffectListItemDialog(context, this);
         modal->setData(*item);
         QObject::connect(modal, &QDialog::accepted, this, [this, model, modal, row]() {
            model->setData(row, modal->data());
            this->_update_cached_data();
         });
         QObject::connect(modal, &QDialog::finished, modal, &QObject::deleteLater);
         modal->setWindowModality(Qt::WindowModality::WindowModal);
         modal->open();
      }
   }

   dovah::form_stub* DKMagicEffectListWidget::costliestMagicEffect() const {
      dovah::magic_effect_list_item_cost_calculator calculator;
      calculator.prepare_game_settings(this->_state.form->stub.get_owning_load_order());

      struct {
         dovah::form_stub* form = nullptr;
         size_t cost = 0;
      } candidate;

      auto count = this->_model->rowCount();
      for (size_t i = 0; i < count; ++i) {
         auto* src = this->_model->data(i);
         if (!src)
            continue;
         auto& dst = calculator.effect;
         dst.area      = src->area;
         dst.duration  = src->duration;
         dst.magnitude = src->magnitude;
         calculator.prepare_effect_form(src->magic_effect);
         if (dst.form_info.flag_24)
            continue;
         
         auto cost = calculator.calculate();
         if (cost > candidate.cost) {
            candidate.form = src->magic_effect;
            candidate.cost = cost;
         }
      }

      return candidate.form;
   }
   QList<dovah::form_stub*> DKMagicEffectListWidget::magicEffects() const {
      QList<dovah::form_stub*> out;

      auto count = this->_model->rowCount();
      for (size_t i = 0; i < count; ++i) {
         auto* data = this->_model->data(i);
         if (!data)
            continue;
         auto* stub = data->magic_effect;
         if (stub)
            out.push_back(stub);
      }

      return out;
   }

   const DKMagicEffectListModelItem* DKMagicEffectListWidget::effect(size_t i) const {
      return this->_model->data(i);
   }
   size_t DKMagicEffectListWidget::effectCount() const {
      return this->_model->rowCount();
   }

   void DKMagicEffectListWidget::autoCalc(AutoCalcData& dst) const {
      dst = {};
      if (!this->_state.form)
         return;

      dovah::magic_effect_list_item_cost_calculator calculator;
      calculator.prepare_game_settings(this->_state.form->stub.get_owning_load_order());

      auto count = this->_model->rowCount();
      for (size_t i = 0; i < count; ++i) {
         auto* src = this->_model->data(i);
         if (!src)
            continue;
         auto& calc_info = calculator.effect;
         calc_info.area      = src->area;
         calc_info.duration  = src->duration;
         calc_info.magnitude = src->magnitude;
         calculator.prepare_effect_form(src->magic_effect);
         if (calc_info.form_info.flag_24)
            continue;

         auto cost = (int32_t) calculator.calculate(); // the CK sums as integers; so too must we
         dst.cost += cost;

         if (dst.charge_time < calc_info.form_info.charge_time)
            dst.charge_time = calc_info.form_info.charge_time;
      }
   }
#endif

#if !defined(QT_PLUGIN)
   void DKMagicEffectListWidget::_new_effect() {
      auto* model = this->_model;
      if (!model || !model->canInsertRows(1))
         return;
      this->openCreateEffectModal();
   }
   void DKMagicEffectListWidget::_edit_effect() {
      this->openEditEffectModal();
   }
   void DKMagicEffectListWidget::_delete_effects() {
      auto* model = this->_model;
      auto* sm    = this->_subwidgets.view->selectionModel();
      if (!this->_state.form || !model || !sm)
         return;
      auto sel = sm->selection();
      if (sel.empty())
         return;
      for (QItemSelectionRange& range : sel) {
         int first = range.topLeft().row();
         int last  = range.bottomRight().row();
         model->removeRows(first, last - first + 1);
      }
   }

   void DKMagicEffectListWidget::_update_cached_data() {
      #if !defined(QT_PLUGIN)
      this->_state.cached = {};

      dovah::magic_effect_list_item_cost_calculator calculator;
      calculator.prepare_game_settings(this->_state.form->stub.get_owning_load_order());

      size_t total_cost = 0;

      size_t size = this->_model->rowCount();
      for (size_t i = 0; i < size; ++i) {
         const auto* src = this->_model->data(i);
         if (!src)
            continue;
         auto& dst = calculator.effect;
         dst.area      = src->area;
         dst.duration  = src->duration;
         dst.magnitude = src->magnitude;
         calculator.prepare_effect_form(src->magic_effect);
         if (dst.form_info.flag_24)
            continue;
         total_cost += calculator.calculate();
      }
      this->_state.cached.auto_calculated_cost = total_cost;

      #endif
   }
   void DKMagicEffectListWidget::_update_can_add_effect() {
      this->_context_menu.actions.add->setEnabled(this->_model->canInsertRows());
   }
#endif