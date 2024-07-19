#include "./DKFormInventory.h"
#include <limits>
#include <QBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QRadioButton>
#include "./DKHeaderView.h"
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/data/all_carryable_form_types.h"
   #include "dovah/forms/components/leveled_list.h"
   #include "dovah/forms/LeveledItem.h"
   #include "dovah/utils/leveled_list_preview.h"
   #include "./widget-dialogs/DKFormInventoryPreviewDialog.h"
   #include "./widget-models/DKFormInventoryModel.h"
#endif

#if defined(QT_DESIGNER_LIB)
   #include <QAbstractItemModel>
   class _DummyModel : public QAbstractItemModel {
      public:
         using QAbstractItemModel::QAbstractItemModel;

         virtual int columnCount(const QModelIndex& item) const override final {
            return 5;
         }
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
            if (orientation != Qt::orientation::Horizontal)
               return {};
            if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
               return {};
            switch (section) {
               case 0:
                  return tr("Count", "column header");
               case 1:
                  return tr("Form Editor ID", "column header");
               case 2:
                  return tr("Owner", "column header");
               case 3:
                  return tr("Health", "column header");
               case 4:
                  return tr("Value", "column header");
            }
            return {};
         }
   };
#endif

DKFormInventory::DKFormInventory(QWidget* parent) : QWidget(parent) {
   auto* layout = new QBoxLayout(QBoxLayout::Direction::TopToBottom, this);
   layout->setContentsMargins(0, 0, 0, 0);
   this->setLayout(layout);

   #if !defined(QT_DESIGNER_LIB)
      this->_model = new DKFormInventoryModel(this);
   #endif

   this->_subwidgets.current_item = new DKFormPicker(this);
   this->_subwidgets.current_count = new QSpinBox(this);
   this->_subwidgets.current_health = new QDoubleSpinBox(this);
   //
   #if !defined(QT_DESIGNER_LIB)
      for (auto ft : dovah::all_carryable_form_types)
         this->_subwidgets.current_item->addAllowedFormType(ft);
   #endif
   this->_subwidgets.current_count->setRange(0, std::numeric_limits<uint16_t>::max());
   this->_subwidgets.current_health->setRange(0, std::numeric_limits<float>::max());

   {
      auto* view = this->_subwidgets.view = new QTableView(this);
      #if !defined(QT_DESIGNER_LIB)
         view->setModel(this->_model);
      #else
         view->setModel(new _DummyModel(view));
      #endif
      {
         view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
         view->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
         view->setCornerButtonEnabled(false);
         view->setAcceptDrops(false);

         if (auto* vh = view->verticalHeader()) {
            vh->setSectionResizeMode(QHeaderView::ResizeToContents);
            vh->setVisible(false);
         }

         auto* header = new DKHeaderView(Qt::Horizontal, view);
         header->setFlexResizeEnabled(true);
         view->setHorizontalHeader(header);
         //
         auto metrics = QFontMetrics(view->font());
         header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
         header->setMinimumSectionSize(2);
         header->setColumnFlex(0, 0, 0, metrics.boundingRect("999").width() * 1.5F + 4);
         header->setColumnFlex(1, 2, 0);
         header->setColumnFlex(2, 0, 0, metrics.boundingRect("100%").width() * 1.5F + 4);
         header->setColumnFlex(3, 1, 0);
         header->setColumnFlex(4, 0, 0, metrics.boundingRect("99999").width() * 1.5F + 4);
         header->setSectionResizeMode(0, QHeaderView::Interactive);
         header->setSectionResizeMode(1, QHeaderView::Interactive);
         header->setSectionResizeMode(2, QHeaderView::Interactive);
         header->setSectionResizeMode(3, QHeaderView::Interactive);
         header->setSectionResizeMode(4, QHeaderView::Interactive);
         header->setStretchLastSection(false);
      }

      auto* groupbox = this->_subwidgets.current_owner = new QGroupBox(this);
      auto* layout   = new QGridLayout(groupbox);
      groupbox->setLayout(layout);

      this->_subwidgets.current_owner_actor   = new DKFormPicker(groupbox);
      this->_subwidgets.current_owner_faction = new DKFormPicker(groupbox);
      this->_subwidgets.current_owner_global  = new DKFormPicker(groupbox);
      this->_subwidgets.current_owner_rank    = new QSpinBox(groupbox);
      //
      this->_subwidgets.current_owner_actor->setAllowedFormType(dovah::form_type::actor_base);
      this->_subwidgets.current_owner_faction->setAllowedFormType(dovah::form_type::faction);
      this->_subwidgets.current_owner_global->setAllowedFormType(dovah::form_type::global);
      this->_subwidgets.current_owner_rank->setRange(0, std::numeric_limits<uint16_t>::max());

      auto* radio_actor   = this->_subwidgets.current_owner_type_actor = new QRadioButton(tr("NPC:"), groupbox);
      auto* radio_faction = this->_subwidgets.current_owner_type_faction = new QRadioButton(tr("Faction:"), groupbox);
      #if !defined(QT_DESIGNER_LIB)
         QObject::connect(radio_actor,   &QRadioButton::toggled, this, &_writeEntryToModel);
         QObject::connect(radio_faction, &QRadioButton::toggled, this, &_writeEntryToModel);
      #endif

      layout->addWidget(radio_actor,   0, 0);
      layout->addWidget(this->_subwidgets.current_owner_actor, 0, 1);

      auto* v_line = new QFrame(groupbox);
      v_line->setFrameShape(QFrame::VLine);
      v_line->setFrameShadow(QFrame::Sunken);
      layout->addWidget(v_line, 0, 1, 4, 1);

      layout->addWidget(radio_faction, 2, 0);
      layout->addWidget(this->_subwidgets.current_owner_faction, 2, 1);

      {
         auto* label = new QLabel(tr("Global:"), groupbox);
         layout->addWidget(label, 0, 2);
         layout->addWidget(this->_subwidgets.current_owner_global, 0, 3);
         label->setBuddy(this->_subwidgets.current_owner_global);
      }
      {
         auto* label = new QLabel(tr("Rank:"), groupbox);
         layout->addWidget(label, 2, 2);
         layout->addWidget(this->_subwidgets.current_owner_rank, 2, 3);
         label->setBuddy(this->_subwidgets.current_owner_rank);
      }
   }
   {  // Preview container
      auto* container = this->_subwidgets.preview_container = new QWidget(this);

      this->_subwidgets.preview_button = new QPushButton(tr("Preview Calculated Result"), container);

      this->_subwidgets.preview_level_label = new QLabel(tr("Level:"), container);
      this->_subwidgets.preview_level = new QSpinBox(container);
      this->_subwidgets.preview_level_label->setBuddy(this->_subwidgets.preview_level);
      this->_subwidgets.preview_level->setRange(0, std::numeric_limits<uint16_t>::max());

      auto* layout = new QHBoxLayout(container);
      container->setLayout(layout);
      layout->addWidget(this->_subwidgets.preview_container);
      layout->addWidget(this->_subwidgets.preview_level_label);
      layout->addWidget(this->_subwidgets.preview_level);
      layout->setStretch(0, 1);

      container->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      container->setFocusProxy(this->_subwidgets.preview_button);
      this->setTabOrder(this->_subwidgets.preview_button, this->_subwidgets.preview_level);
   }

   auto* deets = this->_subwidgets.details_pane = new QWidget(this);
   {
      auto* layout = new QGridLayout(deets);
      layout->setContentsMargins(0, 0, 0, 0);
      deets->setLayout(layout);

      {
         auto* label = new QLabel(tr("Object:"), deets);
         layout->addWidget(label, 0, 0);
         layout->addWidget(this->_subwidgets.current_item, 1, 0);
         label->setBuddy(this->_subwidgets.current_item);
      }
      {
         auto* label = new QLabel(tr("Count:"), deets);
         layout->addWidget(label, 0, 1);
         layout->addWidget(this->_subwidgets.current_count, 1, 1);
         label->setBuddy(this->_subwidgets.current_count);
      }
      {
         auto* label = new QLabel(tr("Health:"), deets);
         layout->addWidget(label, 0, 2);
         layout->addWidget(this->_subwidgets.current_health, 1, 2);
         label->setBuddy(this->_subwidgets.current_health);
      }
      layout->addWidget(this->_subwidgets.current_owner, 2, 0, 1, 3);

      layout->setColumnStretch(0, 2);
      layout->setColumnStretch(1, 1);
      layout->setColumnStretch(2, 1);
   }

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(this->_subwidgets.view);

   #if !defined(QT_DESIGNER_LIB)
   QObject::connect(this->_subwidgets.preview_button, &QPushButton::clicked, this, &DKFormInventory::_preview);
   //
   QObject::connect(this->_subwidgets.view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &_pullEntryFromModel);
   //
   QObject::connect(this->_subwidgets.current_item,           &DKFormPicker::formChanged,                  this, &_writeEntryToModel);
   QObject::connect(this->_subwidgets.current_count,          QOverload<int>::of(&QSpinBox::valueChanged), this, &_writeEntryToModel);
   QObject::connect(this->_subwidgets.current_health,         QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &_writeEntryToModel);
   QObject::connect(this->_subwidgets.current_owner_actor,    &DKFormPicker::formChanged, this, &_writeEntryToModel);
   QObject::connect(this->_subwidgets.current_owner_faction,  &DKFormPicker::formChanged, this, &_writeEntryToModel);
   QObject::connect(this->_subwidgets.current_owner_global,   &DKFormPicker::formChanged, this, &_writeEntryToModel);
   QObject::connect(this->_subwidgets.current_owner_rank,     QOverload<int>::of(&QSpinBox::valueChanged), this, &_writeEntryToModel);
   #endif

   this->_rebuildLayout();
}

void DKFormInventory::setOrientation(Qt::Orientation v) {
   if (this->_state.orientation == v)
      return;
   this->_state.orientation = v;
   this->_rebuildLayout();
}

#if !defined(QT_DESIGNER_LIB)
void DKFormInventory::initializeFrom(const dovah::loaded_forms::components::container_data& component) {
   this->_model->importFrom(component);
}
void DKFormInventory::commitTo(dovah::loaded_forms::components::container_data& component, dovah::loaded_forms::Form& component_containing_form) {
   this->_model->commitTo(component_containing_form, component);
}
#endif

#if !defined(QT_DESIGNER_LIB)
void DKFormInventory::_preview() {
   std::vector<dovah::leveled_list_preview::entry> results;
   {
      auto level = this->_subwidgets.preview_level->value();

      dovah::leveled_list_preview preview;
      preview.input_level  = level;
      preview.player_level = level;
      bool game_settings_prepped = false;

      size_t size = this->_model->rowCount();
      for (size_t i = 0; i < size; ++i) {
         const auto* entry = this->_model->data(i);
         if (!entry)
            break;

         if (!entry->form)
            continue;
         if (entry->count == 0)
            continue;

         if (entry->form->form_type == dovah::form_type::leveled_item) {
            auto loaded = entry->form->load().ptr_cast<dovah::loaded_forms::LeveledItem>();
            if (loaded) {
               auto& ll = loaded->leveled_list_data;
               if (!game_settings_prepped) {
                  game_settings_prepped = true;
                  preview.prepare_game_settings(entry->form->get_owning_load_order(), ll);
               }
               preview.use_special_loot_formula = ll.flags & dovah::loaded_forms::components::leveled_list::flag::special_loot;

               auto ll_results = preview.generate(ll);
               for (const auto& src : ll_results) {
                  bool coalesced = false;
                  for (auto& dst : results) {
                     if (dst.form != src.form)
                        continue;
                     if (dst.extra.has_value() != src.extra.has_value())
                        continue;
                     if (src.extra.has_value()) {
                        auto& a_ex = dst.extra.value();
                        auto& b_ex = src.extra.value();
                        if (a_ex.health != b_ex.health)
                           continue;
                        if (a_ex.owner != b_ex.owner)
                           continue;
                     }

                     dst.count += src.count;
                     coalesced = true;
                     break;
                  }
                  if (coalesced)
                     continue;
                  results.push_back(src);
               }
            }
         }
         results.push_back(dovah::leveled_list_preview::entry{
            .form  = entry->form,
            .count = entry->count,
            .extra = dovah::leveled_list_preview::entry_extra{
               .health = entry->health,
               .owner  = entry->ownership.owner,
            }
         });
      }
   }

   auto* window = new DKFormInventoryPreviewDialog(this);
   window->setContents(results);
   window->show();
}
void DKFormInventory::_pullEntryFromModel() {
   auto blockers = std::array{
      QSignalBlocker(this->_subwidgets.current_item),
      QSignalBlocker(this->_subwidgets.current_count),
      QSignalBlocker(this->_subwidgets.current_health),
      QSignalBlocker(this->_subwidgets.current_owner_type_actor),
      QSignalBlocker(this->_subwidgets.current_owner_type_faction),
      QSignalBlocker(this->_subwidgets.current_owner_actor),
      QSignalBlocker(this->_subwidgets.current_owner_faction),
      QSignalBlocker(this->_subwidgets.current_owner_global),
      QSignalBlocker(this->_subwidgets.current_owner_rank),
   };

   QModelIndex qmi;
   if (auto* sel_model = this->_subwidgets.view->selectionModel()) {
      auto rows = sel_model->selectedRows();
      if (!rows.empty())
         qmi = rows[0];
   }
   const auto* data = this->_model->data(qmi.row());
   bool enable = data != nullptr;

   this->_subwidgets.current_item->setEnabled(enable);
   this->_subwidgets.current_count->setEnabled(enable);
   this->_subwidgets.current_health->setEnabled(enable);
   this->_subwidgets.current_owner->setEnabled(enable);
   if (!data) {
      return;
   }

   this->_subwidgets.current_item->setFormStub(data->form);
   this->_subwidgets.current_count->setValue(data->count);

   if (data->form) {
      bool has_health = false;
      bool has_owner = true;
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
      this->_subwidgets.current_health->setEnabled(has_health);
      this->_subwidgets.current_owner->setEnabled(has_owner);
   } else {
      this->_subwidgets.current_health->setEnabled(false);
      this->_subwidgets.current_owner->setEnabled(false);
   }

   this->_subwidgets.current_health->setValue(data->health * DKFormInventoryModel::health_display_mult);
   {
      auto  type = dovah::form_type::none;
      auto* stub = data->ownership.owner;
      if (stub)
         type = stub->form_type;
      //
      switch (type) {
         case dovah::form_type::actor_base:
            this->_subwidgets.current_owner_type_actor->setChecked(true);
            this->_subwidgets.current_owner_actor->setFormStub(stub);
            break;
         case dovah::form_type::faction:
            this->_subwidgets.current_owner_type_faction->setChecked(true);
            this->_subwidgets.current_owner_faction->setFormStub(stub);
            break;
         default:
            this->_subwidgets.current_owner_type_actor->setChecked(true);
            this->_subwidgets.current_owner_actor->setFormStub(nullptr);
            break;
      }
   }
   this->_subwidgets.current_owner_global->setFormStub(data->ownership.global);
   this->_subwidgets.current_owner_rank->setValue(data->ownership.rank);
}
void DKFormInventory::_writeEntryToModel() {
   auto* view      = this->_subwidgets.view;
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

   DKFormInventoryModel::InventoryObject dst = *src;
   dst.form  = this->_subwidgets.current_item->formStub();
   dst.count = this->_subwidgets.current_count->value();
   dst.health = this->_subwidgets.current_health->value() / DKFormInventoryModel::health_display_mult;
   if (this->_subwidgets.current_owner_type_actor->isChecked()) {
      dst.ownership.owner = this->_subwidgets.current_owner_actor->formStub();
   } else {
      dst.ownership.owner = this->_subwidgets.current_owner_faction->formStub();
   }
   dst.ownership.global = this->_subwidgets.current_owner_global->formStub();
   dst.ownership.rank   = this->_subwidgets.current_owner_rank->value();

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
#endif
void DKFormInventory::_rebuildLayout() {
   auto* layout = dynamic_cast<QBoxLayout*>(this->layout());
   if (!layout)
      return;
   if (this->_state.orientation == Qt::Orientation::Horizontal) {
      layout->setDirection(QBoxLayout::Direction::LeftToRight);
   } else {
      layout->setDirection(QBoxLayout::Direction::TopToBottom);
   }
}