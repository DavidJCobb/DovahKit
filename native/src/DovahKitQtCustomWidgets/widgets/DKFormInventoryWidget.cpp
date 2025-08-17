#include "./DKFormInventoryWidget.h"
#include <array>
#include <limits>
#include <QBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QRadioButton>
#include "./DKHeaderView.h"
#if !defined(QT_PLUGIN)
   #include "dovah/data/all_carryable_form_types.h"
   #include "dovah/data/all_item_form_types.h"
   #include "dovah/forms/components/leveled_list.h"
   #include "dovah/forms/LeveledItem.h"
   #include "dovah/utils/leveled_list_preview.h"
   #include "./widget-dialogs/DKLeveledListPreviewDialog.h"
   #include "./widget-models/DKFormInventoryModel.h"
#endif

#if defined(QT_PLUGIN)
   #include <QAbstractItemModel>

   // Dummy model, so that the widget displays the right tableview column 
   // headers when placed and previewed in Qt Designer.
   class _DummyModel final : public QAbstractItemModel {
      public:
         using QAbstractItemModel::QAbstractItemModel;

         #pragma region No-ops
            virtual QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override {
               return {};
            }
            virtual QModelIndex parent(const QModelIndex& child) const override {
               return {};
            }
            virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
               return {};
            }
         #pragma endregion
            
         virtual int rowCount(const QModelIndex& item) const override {
            return 0;
         }
         virtual int columnCount(const QModelIndex& item) const override {
            if (!this->_allow_extra_data) {
               return 3;
            }
            return 5;
         }
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
            if (orientation != Qt::Orientation::Horizontal)
               return {};
            if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
               return {};
            if (!this->_allow_extra_data) {
               if (section >= 2)
                  section += 2;
            }
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

         bool allowsExtraData() const {
            return this->_allow_extra_data;
         }
         void setAllowsExtraData(bool v) {
            if (v == this->_allow_extra_data)
               return;
            if (v) {
               this->beginInsertColumns({}, 2, 3);
            } else {
               this->beginRemoveColumns({}, 2, 3);
            }
            this->_allow_extra_data = v;
            if (v) {
               this->endInsertColumns();
            } else {
               this->endRemoveColumns();
            }
         }

      protected:
         bool _allow_extra_data = true;
   };
#endif

DKFormInventoryWidget::DKFormInventoryWidget(QWidget* parent) : QWidget(parent) {
   auto& ui = this->_subwidgets;

   //
   // Create widgets and establish layout:
   //
   ui.view = new QTableView(this);
   {
      auto* deets  = ui.details_pane = new QWidget(this);
      auto* layout = new QGridLayout(deets);
      deets->setLayout(layout);
      layout->setContentsMargins(0, 0, 0, 0);

      size_t column_count = 4;

      ui.current_item   = new DKFormPicker(this);
      ui.current_count  = new QSpinBox(this);
      ui.current_health = new QDoubleSpinBox(this);
      {
         auto* label  = new QLabel(tr("Object:"), deets);
         auto* widget = ui.current_item;
         label->setBuddy(widget);
         layout->addWidget(label,  0, 0);
         layout->addWidget(widget, 0, 1, 1, column_count - 1);
      }
      {
         auto* label  = new QLabel(tr("Count:"), deets);
         auto* widget = ui.current_count;
         label->setBuddy(widget);
         layout->addWidget(label,  1, 0);
         layout->addWidget(widget, 1, 1);
      }
      {
         auto* label  = new QLabel(tr("Health:"), deets);
         auto* widget = ui.current_health;
         label->setBuddy(widget);
         layout->addWidget(label,  1, 2, Qt::AlignmentFlag::AlignRight);
         layout->addWidget(widget, 1, 3);
         ui.labels.current_health = label;
      }
      layout->setColumnStretch(1, 1);
      layout->setColumnStretch(3, 1);

      // Ownership
      auto* owner_group = ui.current_owner = new QGroupBox(tr("Owner:"), this);
      ui.current_owner_type_actor   = new QRadioButton(tr("NPC:"),     owner_group);
      ui.current_owner_type_faction = new QRadioButton(tr("Faction:"), owner_group);
      ui.current_owner_actor        = new DKFormPicker(owner_group);
      ui.current_owner_faction      = new DKFormPicker(owner_group);
      ui.current_owner_global       = new DKFormPicker(owner_group);
      ui.current_owner_rank         = new QSpinBox(owner_group);
      {
         auto* layout = new QGridLayout(owner_group);
         owner_group->setLayout(layout);
         
         auto* v_line = new QFrame(owner_group);
         v_line->setFrameShape(QFrame::VLine);
         v_line->setFrameShadow(QFrame::Sunken);

         layout->addWidget(ui.current_owner_type_actor,   0, 0);
         layout->addWidget(ui.current_owner_actor,        1, 0);
         {
            auto* label  = new QLabel(tr("Global:"), owner_group);
            auto* widget = ui.current_owner_global;
            label->setBuddy(widget);
            layout->addWidget(label,  2, 0);
            layout->addWidget(widget, 3, 0);
         }
         layout->addWidget(v_line, 0, 1, 4, 1);
         layout->addWidget(ui.current_owner_type_faction, 0, 2);
         layout->addWidget(ui.current_owner_faction,      1, 2);
         {
            auto* label  = new QLabel(tr("Required rank:"), owner_group);
            auto* widget = ui.current_owner_rank;
            label->setBuddy(widget);
            layout->addWidget(label,  2, 2);
            layout->addWidget(widget, 3, 2);
         }
      }
      layout->addWidget(owner_group, 3, 0, 1, column_count);
      
      {  // Preview container
         auto* container = ui.preview_container = new QWidget(this);
         auto* layout = new QHBoxLayout(container);
         layout->setContentsMargins(0, 0, 0, 0);
         container->setLayout(layout);

         ui.preview_button = new QPushButton(tr("Preview Calculated Result"), container);
         ui.preview_level  = new QSpinBox(container);

         layout->addWidget(ui.preview_button, 1);
         {
            auto* label  = new QLabel(tr("Preview Level:"), container);
            auto* widget = ui.preview_level;
            label->setBuddy(widget);
            layout->addWidget(label);
            layout->addWidget(widget);
         }
         layout->addStretch(1);

         container->setFocusPolicy(Qt::FocusPolicy::TabFocus);
         container->setFocusProxy(ui.preview_button);
         this->setTabOrder(ui.preview_button, ui.preview_level);
      }
      layout->addWidget(ui.preview_container, 4, 0, 1, column_count);
   }
   //
   auto* layout = new QBoxLayout(QBoxLayout::Direction::TopToBottom, this);
   layout->setContentsMargins(0, 0, 0, 0);
   this->setLayout(layout);
   layout->addWidget(ui.view);
   layout->addWidget(ui.details_pane);

   //
   // Focus and tab order:
   //
   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(ui.view);
   QWidget::setTabOrder(ui.view, ui.details_pane);
   //
   ui.details_pane->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   ui.details_pane->setFocusProxy(ui.current_item);
   {
      auto widgets = std::array<QWidget*, 10>{
         ui.current_item,
         ui.current_count,
         ui.current_health,
         ui.current_owner_type_actor,
         ui.current_owner_actor,
         ui.current_owner_global,
         ui.current_owner_type_faction,
         ui.current_owner_faction,
         ui.current_owner_rank,
         ui.preview_container
      };
      for (size_t i = 0; i + 1 < widgets.size(); ++i)
         QWidget::setTabOrder(widgets[i], widgets[i + 1]);
   }

   #pragma region "What's this?" text
      //: "What's this?" text for changing an item's health.
      ui.current_health->setWhatsThis(tr("For weapons and armor, the condition they're in. Values above 100% produce the same effects as tempering the item using the Smithing skill.", "what's this?"));

      //: "What's this?" text for changing an owned item's required rank.
      ui.current_owner_rank->setWhatsThis(tr("If the item's owner is a faction, then NPCs must have this rank or greater within the faction in order to share in ownership of the item.", "what's this?"));

      //: "What's this?" text for changing an owned item's requirement global.
      ui.current_owner_global->setWhatsThis(tr("Unused. The ability to associate a Global with per-actor ownership was introduced in Fallout 3, but was unused even in that game, and its purpose -- beyond limiting when or how the actor is considered the item's owner, somehow -- is unknown.", "what's this?"));

      //: "What's this?" text for the "Preview Calculated Result" button.
      ui.preview_button->setWhatsThis(tr("Given the level of a hypothetical player or encounter zone, compute the results of any LeveledItems in this container, and show the container's (potential) resulting inventory.", "what's this?"));
   #pragma endregion

   //
   // Create model and configure widgets:
   //
   #if !defined(QT_PLUGIN)
      this->_model = new DKFormInventoryModel(this);
   #endif
   //
   #if !defined(QT_PLUGIN)
      for (auto ft : dovah::all_carryable_form_types)
         ui.current_item->addAllowedFormType(ft);
   #endif
   ui.current_count->setRange(0, std::numeric_limits<uint16_t>::max());
   ui.current_count->setValue(1);
   ui.current_count->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
   ui.current_health->setRange(0, std::numeric_limits<float>::max());
   ui.current_health->setValue(100);
   ui.current_health->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
   {  // Layout tweaks for spinboxes
      //
      // Spinboxes define their size hint and minimum size hint to ensure that 
      // nothing that would ever be displayed inside would be truncated. This 
      // means that our "health" spinbox has a larger minimum size than the 
      // "count" spinbox (since it has a maximum of FLT_MAX i.e. eleventy-two 
      // million or whatever).
      // 
      // However, "health" values will generally be near 100, and I'd rather 
      // have the column widths be balanced between the two... so let's force 
      // the "health" spinbox to borrow the "count" spinbox's minimum size.
      // 
      ui.current_health->setMinimumSize(ui.current_count->minimumSizeHint());
      //
      // Let's also ensure that the sizeHint() isn't what's treated as the 
      // minimum (which is the default size policy).
      //
      ui.current_count->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
      ui.current_health->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
   }
   //
   ui.current_owner_actor->setAllowedFormType(dovah::form_type::actor_base);
   ui.current_owner_faction->setAllowedFormType(dovah::form_type::faction);
   ui.current_owner_global->setAllowedFormType(dovah::form_type::global);
   ui.current_owner_rank->setRange(0, std::numeric_limits<uint16_t>::max());
   ui.current_owner_rank->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
   //
   ui.preview_level->setRange(0, std::numeric_limits<uint16_t>::max());
   ui.preview_level->setValue(1);
   ui.preview_level->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
   //
   {
      auto* view = ui.view;
      #if !defined(QT_PLUGIN)
         view->setModel(this->_model);
      #else
         view->setModel(new _DummyModel(view));
      #endif
      {
         view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
         view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
         view->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
         view->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
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
         header->setColumnFlex(0, 0, 0, metrics.boundingRect("99999").width() * 1.5F + 4);
         header->setColumnFlex(1, 2, 0);
         header->setColumnFlex(2, 1, 0);
         header->setColumnFlex(3, 0, 0, metrics.boundingRect("100%").width() * 1.5F + 4);
         header->setColumnFlex(4, 0, 0, metrics.boundingRect("99999").width() * 1.5F + 4);
         header->setSectionResizeMode(0, QHeaderView::Interactive);
         header->setSectionResizeMode(1, QHeaderView::Interactive);
         header->setSectionResizeMode(2, QHeaderView::Interactive);
         header->setSectionResizeMode(3, QHeaderView::Interactive);
         header->setSectionResizeMode(4, QHeaderView::Interactive);
         header->setStretchLastSection(false);
      }

      #if !defined(QT_PLUGIN)
         QObject::connect(ui.current_owner_type_actor,   &QRadioButton::toggled, this, &DKFormInventoryWidget::_writeEntryToModel);
         QObject::connect(ui.current_owner_type_faction, &QRadioButton::toggled, this, &DKFormInventoryWidget::_writeEntryToModel);
      #endif
   }

   #if !defined(QT_PLUGIN)
   QObject::connect(this->_subwidgets.preview_button, &QPushButton::clicked, this, &DKFormInventoryWidget::_preview);
   //
   QObject::connect(this->_subwidgets.view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DKFormInventoryWidget::_pullEntryFromModel);
   //
   QObject::connect(this->_subwidgets.current_item,           &DKFormPicker::formChanged,                  this, &DKFormInventoryWidget::_writeEntryToModel);
   QObject::connect(this->_subwidgets.current_count,          QOverload<int>::of(&QSpinBox::valueChanged), this, &DKFormInventoryWidget::_writeEntryToModel);
   QObject::connect(this->_subwidgets.current_health,         QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DKFormInventoryWidget::_writeEntryToModel);
   QObject::connect(this->_subwidgets.current_owner_actor,    &DKFormPicker::formChanged, this, &DKFormInventoryWidget::_writeEntryToModel);
   QObject::connect(this->_subwidgets.current_owner_faction,  &DKFormPicker::formChanged, this, &DKFormInventoryWidget::_writeEntryToModel);
   QObject::connect(this->_subwidgets.current_owner_global,   &DKFormPicker::formChanged, this, &DKFormInventoryWidget::_writeEntryToModel);
   QObject::connect(this->_subwidgets.current_owner_rank,     QOverload<int>::of(&QSpinBox::valueChanged), this, &DKFormInventoryWidget::_writeEntryToModel);
   #endif

   this->_rebuildLayout();
   #if !defined(QT_PLUGIN)
      this->_pullEntryFromModel(); // set initial control enable states
   #endif
}

bool DKFormInventoryWidget::allowsExtraData() const noexcept {
   #if defined(QT_PLUGIN)
      return this->_state.allow_extra_data;
   #else
      if (!this->_model)
         return true;
      return this->_model->allowsExtraData();
   #endif
}
void DKFormInventoryWidget::setAllowsExtraData(bool v) {
   #if defined(QT_PLUGIN)
      this->_state.allow_extra_data = v;
      if (auto* model = dynamic_cast<_DummyModel*>(this->_subwidgets.view->model())) {
         model->setAllowsExtraData(v);
      }
   #else
      this->_model->setAllowsExtraData(v);
   #endif
   this->_rebuildLayout();
}

bool DKFormInventoryWidget::allowsPseudoItems() const noexcept {
   #if defined(QT_PLUGIN)
      return this->_state.allow_pseudo_Items;
   #else
      if (!this->_model)
         return true;
      return this->_model->allowsPseudoItems();
   #endif

}
void DKFormInventoryWidget::setAllowsPseudoItems(bool v) {
   #if defined(QT_PLUGIN)
      this->_state.allow_pseudo_Items = v;
   #else
      this->_model->setAllowsPseudoItems(v);
      #if !defined(QT_PLUGIN)
      {
         auto* widget = this->_subwidgets.current_item;
         auto* form   = widget->formStub();
         const auto blocker = QSignalBlocker(widget);
         widget->setAllowedFormTypes({});
         if (v) {
            for (auto ft : dovah::all_carryable_form_types)
               widget->addAllowedFormType(ft);
         } else {
            for (auto ft : dovah::all_item_form_types)
               widget->addAllowedFormType(ft);
         }
         widget->setFormStub(form);
      }
      #endif
   #endif
}

void DKFormInventoryWidget::setOrientation(Qt::Orientation v) {
   if (this->_state.orientation == v)
      return;
   this->_state.orientation = v;
   this->_rebuildLayout();
}

void DKFormInventoryWidget::setShowPreviewWidgets(bool v) {
   if (this->_state.show_preview_widgets == v)
      return;
   this->_state.show_preview_widgets = v;
   this->_rebuildLayout();
}

#if !defined(QT_PLUGIN)
void DKFormInventoryWidget::initializeFrom(const dovah::loaded_forms::components::container_data& component) {
   this->_model->importFrom(component);
}
void DKFormInventoryWidget::commitTo(dovah::loaded_forms::components::container_data& component, dovah::loaded_forms::Form& component_containing_form) {
   this->_model->commitTo(component_containing_form, component);
}
#endif

#if !defined(QT_PLUGIN)
void DKFormInventoryWidget::_preview() {
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
                  continue;
               }
               continue;
            }
            #if _DEBUG
               __debugbreak(); // Why didn't this form load?
            #endif
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

   auto* window = new DKLeveledListPreviewDialog(this);
   window->setContents(results);
   window->show();
}
void DKFormInventoryWidget::_pullEntryFromModel() {
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
void DKFormInventoryWidget::_writeEntryToModel() {
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
void DKFormInventoryWidget::_rebuildLayout() {
   auto* layout = dynamic_cast<QBoxLayout*>(this->layout());
   if (!layout)
      return;
   if (this->_state.orientation == Qt::Orientation::Horizontal) {
      layout->setDirection(QBoxLayout::Direction::LeftToRight);
   } else {
      layout->setDirection(QBoxLayout::Direction::TopToBottom);
   }

   bool allows_extra_data = this->allowsExtraData();
   //
   auto& ui = this->_subwidgets;
   ui.labels.current_health->setVisible(allows_extra_data);
   ui.current_health->setVisible(allows_extra_data);
   ui.current_owner->setVisible(allows_extra_data);
   {
      auto* layout = ui.current_count->parentWidget()->layout();
      if (layout) {
         auto* item = layout->itemAt(layout->indexOf(ui.current_count));
         if (item) {
            Qt::Alignment align = {};
            if (!allows_extra_data) {
               align = Qt::AlignmentFlag::AlignLeft;
            }
            item->setAlignment(align);
         }
      }
   }

   // show_preview_widgets
   ui.preview_container->setVisible(this->_state.show_preview_widgets);
}