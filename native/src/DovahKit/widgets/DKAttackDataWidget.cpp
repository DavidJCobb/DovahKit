#include "./DKAttackDataWidget.h"
#include <array>
#include <limits>
#include <QBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include "./DKHeaderView.h"
#if !defined(QT_DESIGNER_LIB)
   #include "./widget-models/DKAttackDataModel.h"
   #include "dovah/forms/components/attack_data.h"
#endif

namespace {
   constexpr const auto spinbox_alignment = Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter;
}

DKAttackDataWidget::DKAttackDataWidget(QWidget* parent) : QWidget(parent) {
   auto& ui = this->_subwidgets;

   #if !defined(QT_DESIGNER_LIB)
      this->_model = new DKAttackDataModel(this);
   #endif

   #pragma region Create and configure widgets and layout
   {
      auto* layout = new QVBoxLayout(this);
      layout->setContentsMargins(0, 0, 0, 0);

      layout->addWidget(ui.race.container = new QWidget(this));
      layout->addWidget(ui.event_listing.container = new QWidget(this));
      layout->addWidget(ui.event_edit.container = new QWidget(this));

      {  // Attack Race
         auto& ui     = this->_subwidgets.race;
         auto* layout = new QHBoxLayout(ui.container);
         layout->setContentsMargins(0, 0, 0, 0);

         auto* race = ui.picker = new DKFormPicker(this);
         race->setAllowedFormType(dovah::form_type::race);
         race->setAllowNone(true);

         auto* label = new QLabel(tr("Attack Race:"), this);
         label->setBuddy(race);

         layout->addWidget(label);
         layout->addWidget(race);
      }

      {  // Event listing
         auto& ui     = this->_subwidgets.event_listing;
         auto* layout = new QHBoxLayout(ui.container);
         layout->setContentsMargins(0, 0, 0, 0);

         auto* view = ui.view = new QTableView(this);
         #if !defined(QT_DESIGNER_LIB)
            view->setModel(this->_model);
         #endif
         layout->addWidget(view);
         {
            view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
            view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
            view->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
            view->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
            view->setCornerButtonEnabled(false);
            view->setAcceptDrops(false);
            view->setWordWrap(false);

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
            #if !defined(QT_DESIGNER_LIB)
               header->setColumnFlex(DKAttackDataModel::Column::Name,    1, 0, metrics.boundingRect("Event Name Event Name").width() * 1.5F + 4);
               header->setColumnFlex(DKAttackDataModel::Column::Spell,   1, 0, metrics.boundingRect("ReallyCoolFireball01").width() * 1.5F + 4);
               header->setColumnFlex(DKAttackDataModel::Column::Keyword, 1, 0, metrics.boundingRect("CoolSpellKeyword").width() * 1.5F + 4);
               header->setColumnFlex(DKAttackDataModel::Column::AttackChance, 0, 0, metrics.boundingRect("1.000").width() * 1.5F + 4);
               header->setColumnFlex(DKAttackDataModel::Column::DamageMult, 0, 0, metrics.boundingRect("1.000").width() * 1.5F + 4);
               header->setColumnFlex(DKAttackDataModel::Column::Knockdown, 0, 0, metrics.boundingRect("1.000").width() * 1.5F + 4);
               header->setColumnFlex(DKAttackDataModel::Column::RecoveryTime, 0, 0, metrics.boundingRect("1.000").width() * 1.5F + 4);
               header->setColumnFlex(DKAttackDataModel::Column::Stagger, 0, 0, metrics.boundingRect("1.000").width() * 1.5F + 4);
               header->setColumnFlex(DKAttackDataModel::Column::StaminaCostMult, 0, 0, metrics.boundingRect("1.000").width() * 1.5F + 4);
               header->setColumnFlex(DKAttackDataModel::Column::Angle, 0, 0, metrics.boundingRect("360+360").width() * 1.5F + 4);
               for(size_t i = 0; i < DKAttackDataModel::column_count; ++i)
                  header->setSectionResizeMode(i, QHeaderView::Interactive);
            #endif
            header->setStretchLastSection(false);
         }

         auto* buttons_container = new QWidget(this);
         layout->addWidget(buttons_container);
         auto* buttons_layout = new QVBoxLayout(buttons_container);
         buttons_layout->setContentsMargins(0, 0, 0, 0);

         buttons_layout->addWidget(ui.button_create = new QPushButton(tr("Add"),    buttons_container));
         buttons_layout->addWidget(ui.button_delete = new QPushButton(tr("Remove"), buttons_container));
         buttons_layout->addStretch(1);
      }

      {  // Event editing
         auto& ui     = this->_subwidgets.event_edit;
         auto* layout = new QGridLayout(ui.container);
         layout->setContentsMargins(0, 0, 0, 0);

         int row = 0;
         int col = 0;

         // Left col
         {  // Event
            auto* label  = new QLabel(tr("Event:"), this);
            auto* widget = ui.name = new QLineEdit(this);
            label->setBuddy(widget);
            widget->setWhatsThis(tr("The behavior graph event associated with this attack."));
            //
            layout->addWidget(label,  row, col);
            layout->addWidget(widget, row, col + 1);
         }
         ++row;
         {  // Damage Mult
            auto* label  = new QLabel(tr("Damage Mult:"), this);
            auto* widget = ui.damage_mult = new QDoubleSpinBox(this);
            label->setBuddy(widget);
            widget->setAlignment(spinbox_alignment);
            widget->setRange(0, std::numeric_limits<float>::max());
            //
            layout->addWidget(label,  row, col);
            layout->addWidget(widget, row, col + 1, Qt::AlignmentFlag::AlignLeft);
         }
         ++row;
         {  // Attack Chance
            auto* label  = new QLabel(tr("Attack Chance:"), this);
            auto* widget = ui.attack_chance = new QDoubleSpinBox(this);
            label->setBuddy(widget);
            widget->setAlignment(spinbox_alignment);
            widget->setRange(0, std::numeric_limits<float>::max());
            widget->setWhatsThis(tr("The chance that the actor will use this attack."));
            //
            layout->addWidget(label,  row, col);
            layout->addWidget(widget, row, col + 1, Qt::AlignmentFlag::AlignLeft);
         }
         ++row;
         {  // Stagger
            auto* label = new QLabel(tr("Stagger:"), this);
            auto* widget = ui.stagger = new QDoubleSpinBox(this);
            label->setBuddy(widget);
            widget->setAlignment(spinbox_alignment);
            widget->setRange(0, std::numeric_limits<float>::max());
            //
            layout->addWidget(label,  row, col);
            layout->addWidget(widget, row, col + 1, Qt::AlignmentFlag::AlignLeft);
         }
         ++row;
         {  // Recovery Time
            auto* label = new QLabel(tr("Recovery Time:"), this);
            auto* widget = ui.recovery_time = new QDoubleSpinBox(this);
            label->setBuddy(widget);
            widget->setAlignment(spinbox_alignment);
            widget->setRange(0, std::numeric_limits<float>::max());
            //
            layout->addWidget(label,  row, col);
            layout->addWidget(widget, row, col + 1, Qt::AlignmentFlag::AlignLeft);
         }
         ++row;
         {  // Stamina Cost Mult
            auto* label = new QLabel(tr("Stamina Cost Mult:"), this);
            auto* widget = ui.stamina_cost_mult = new QDoubleSpinBox(this);
            label->setBuddy(widget);
            widget->setAlignment(spinbox_alignment);
            widget->setRange(0, std::numeric_limits<float>::max());
            //
            layout->addWidget(label,  row, col);
            layout->addWidget(widget, row, col + 1, Qt::AlignmentFlag::AlignLeft);
         }
         ++row;
         {  // Attack Spell
            auto* label = new QLabel(tr("Attack Spell:"), this);
            auto* widget = ui.attack_spell = new DKFormPicker(this);
            label->setBuddy(widget);
            widget->setAllowedFormTypes({ dovah::form_type::shout, dovah::form_type::spell });
            widget->setAllowNone(true);
            widget->setWhatsThis(tr("A spell that will be applied to targets hit by this attack."));
            //
            layout->addWidget(label, row, col);
            layout->addWidget(widget, row, col + 1);
         }
         ++row;
         {  // Attack Type
            auto* label = new QLabel(tr("Attack Type:"), this);
            auto* widget = ui.attack_type = new DKFormPicker(this);
            label->setBuddy(widget);
            widget->setAllowedFormType(dovah::form_type::keyword);
            widget->setAllowNone(true);
            widget->setWhatsThis(tr("A keyword which identifies the attack type. Used by perks."));
            //
            layout->addWidget(label, row, col);
            layout->addWidget(widget, row, col + 1);
         }

         row = 0;
         col += 2;
         {  // Spacing between columns
            auto* spacer = new QSpacerItem(16, 8, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
            layout->addItem(spacer, row, col);
         }
         ++col;

         // Right col
         {  // Attack Angle
            auto* label = new QLabel(tr("Attack Angle:"), this);
            auto* widget = ui.attack_angle = new QDoubleSpinBox(this);
            label->setBuddy(widget);
            widget->setAlignment(spinbox_alignment);
            widget->setRange(-360, 360);
            widget->setWhatsThis(tr("The direction of the attack relative to the attacking actor. A value of 180 degrees, for example, would cause the attack to travel backwards from the actor."));
            //
            layout->addWidget(label, row, col);
            layout->addWidget(widget, row, col + 1);
         }
         ++row;
         {  // Angle Range
            auto* label = new QLabel(tr("Angle Range:"), this);
            auto* widget = ui.angle_range = new QDoubleSpinBox(this);
            label->setBuddy(widget);
            widget->setAlignment(spinbox_alignment);
            widget->setRange(-360, 360);
            widget->setWhatsThis(tr("The angle tolerance on the attack. For example, a value of 35 means that targets within (+/-)35 degrees of the Attack Angle would be hit by the attack."));
            //
            layout->addWidget(label, row, col);
            layout->addWidget(widget, row, col + 1);
         }
         ++row;
         {  // Knockdown
            auto* label = new QLabel(tr("Knockdown:"), this);
            auto* widget = ui.knockdown = new QDoubleSpinBox(this);
            label->setBuddy(widget);
            widget->setAlignment(spinbox_alignment);
            widget->setRange(0, std::numeric_limits<float>::max());
            //
            layout->addWidget(label, row, col);
            layout->addWidget(widget, row, col + 1);
         }
         ++row;
         {
            auto& flags = ui.flags;
            {
               auto* widget = flags.power = new QCheckBox(tr("Power Attack"), this);
               widget->setWhatsThis(tr("If this is checked, then the attack is a Power Attack: the actor can only perform it if they have enough Stamina, and it will use that amount of Stamina."));
               layout->addWidget(widget, row, col, 1, 2);
            }
            ++row;
            {
               auto* widget = flags.ignore_weapon = new QCheckBox(tr("Ignore Weapon"), this);
               widget->setWhatsThis(tr("If checked, this attack will ignore the damage formula for the actor's weapon."));
               layout->addWidget(widget, row, col, 1, 2);
            }
            ++row;
            {
               auto* widget = flags.bash = new QCheckBox(tr("Bash Attack"), this);
               layout->addWidget(widget, row, col, 1, 2);
            }
            ++row;
            {
               auto* widget = flags.lefthanded = new QCheckBox(tr("Lefthanded Attack"), this);
               layout->addWidget(widget, row, col, 1, 2);
            }
            ++row;
            {
               auto* widget = flags.rotating = new QCheckBox(tr("Rotating Attack"), this);
               widget->setWhatsThis(tr("Indicates whether this attack rotates the actor's character controller (i.e. their movement and physics)."));
               layout->addWidget(widget, row, col, 1, 2);
            }
            ++row;
         }
      }
   }
   #pragma endregion

   #pragma region Focus and tab order
      this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      this->setFocusProxy(ui.race.container);
      QWidget::setTabOrder(ui.race.container, ui.event_listing.container);
      QWidget::setTabOrder(ui.event_listing.container, ui.event_edit.container);
      {
         auto& ui = this->_subwidgets.race;
         ui.container->setFocusPolicy(Qt::FocusPolicy::TabFocus);
         ui.container->setFocusProxy(ui.picker);
      }
      {
         auto& ui = this->_subwidgets.event_listing;
         ui.container->setFocusPolicy(Qt::FocusPolicy::TabFocus);
         ui.container->setFocusProxy(ui.view);
         QWidget::setTabOrder(ui.view, ui.button_create);
         QWidget::setTabOrder(ui.button_create, ui.button_delete);
      }
      {
         auto& ui = this->_subwidgets.event_edit;
         ui.container->setFocusPolicy(Qt::FocusPolicy::TabFocus);
         ui.container->setFocusProxy(ui.name);

         const auto widgets = std::array<QWidget*, 16>{
            ui.name,
            ui.damage_mult,
            ui.attack_chance,
            ui.stagger,
            ui.recovery_time,
            ui.stamina_cost_mult,
            ui.attack_spell,
            ui.attack_type,
            ui.attack_angle,
            ui.angle_range,
            ui.knockdown,
            ui.flags.power,
            ui.flags.ignore_weapon,
            ui.flags.bash,
            ui.flags.lefthanded,
            ui.flags.rotating
         };
         for (size_t i = 0; i + 1 < widgets.size(); ++i) {
            QWidget::setTabOrder(widgets[i], widgets[i + 1]);
         }
      }
   #pragma endregion

   #pragma region Behavior
   #if !defined(QT_DESIGNER_LIB)
   {
      auto* view      = this->_subwidgets.event_listing.view;
      auto* sel_model = view->selectionModel();
      auto* model     = this->_model;

      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, &DKAttackDataWidget::_pull_node_to_ui);
      auto& ui = this->_subwidgets.event_edit;
      QObject::connect(ui.name,                &QLineEdit::editingFinished, this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.damage_mult,         QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.attack_chance,       QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.stagger,             QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.recovery_time,       QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.stamina_cost_mult,   QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.attack_spell,        &DKFormPicker::formChanged, this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.attack_type,         &DKFormPicker::formChanged, this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.attack_angle,        QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.angle_range,         QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.knockdown,           QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.flags.power,         &QCheckBox::toggled, this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.flags.ignore_weapon, &QCheckBox::toggled, this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.flags.bash,          &QCheckBox::toggled, this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.flags.lefthanded,    &QCheckBox::toggled, this, &DKAttackDataWidget::_push_node_from_ui);
      QObject::connect(ui.flags.rotating,      &QCheckBox::toggled, this, &DKAttackDataWidget::_push_node_from_ui);

      QObject::connect(this->_subwidgets.event_listing.button_create, &QPushButton::clicked, this, [this, sel_model, model]() {
         auto qmi = model->create();
         if (!qmi.isValid())
            return;
         auto col = model->columnCount({});
         auto tl  = qmi.siblingAtColumn(0);
         auto br  = qmi.siblingAtColumn(col - 1);
         sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
      });
      QObject::connect(this->_subwidgets.event_listing.button_delete, &QPushButton::clicked, this, [this, sel_model, model]() {
         size_t row;
         {
            auto rows = sel_model->selectedRows();
            if (rows.isEmpty())
               return;
            row = rows[0].row();
         }
         model->deleteItems(row, 1);
      });
   }
   #endif
   #pragma endregion
}

void DKAttackDataWidget::setShowsAttackRace(bool v) {
   if (this->showsAttackRace() == v)
      return;
   this->_state.show_attack_race = v;

   auto* layout = (QBoxLayout*)this->layout();
   auto* widget = this->_subwidgets.race.container;
   if (v) {
      layout->insertWidget(0, widget);
   } else {
      layout->removeWidget(widget);
   }
}

#if !defined(QT_DESIGNER_LIB)
   void DKAttackDataWidget::initializeFrom(const dovah::loaded_forms::components::attack_data& src) {
      {
         auto* widget  = this->_subwidgets.race.picker;
         auto  blocker = QSignalBlocker(widget);
         widget->setFormStub(src.race.get_form_stub());
      }

      std::vector<DKAttackDataModelNode> nodes;
      {
         using flag = dovah::loaded_forms::components::attack_data::flag;

         size_t size = src.attacks.size();
         nodes.resize(size);
         for (size_t i = 0; i < size; ++i) {
            auto& src_item = src.attacks[i];
            auto& dst_item = nodes[i];

            dst_item.event_name        = QString::fromStdString(src_item.event);
            dst_item.attack_chance     = src_item.attack_chance;
            dst_item.damage_mult       = src_item.damage_mult;
            dst_item.keyword           = src_item.keyword.get_form_stub();
            dst_item.knockdown         = src_item.knockdown;
            dst_item.spell             = src_item.attack_spell.get_form_stub();
            dst_item.recovery_time     = src_item.recovery_time;
            dst_item.stagger           = src_item.stagger;
            dst_item.stamina_cost_mult = src_item.stamina_mult;
            dst_item.angles.direction  = src_item.attack_angle;
            dst_item.angles.range      = src_item.strike_angle;
            dst_item.flags = {
               .bash = (bool)(src_item.flags & flag::bash_attack),
               .ignore_weapon = (bool)(src_item.flags & flag::ignore_weapon),
               .lefthanded = (bool)(src_item.flags & flag::left_attack),
               .power = (bool)(src_item.flags & flag::power_attack),
               .rotating = (bool)(src_item.flags & flag::rotating_attack),
            };
         }
      }

      this->_model->overwriteAllItems(nodes);
      this->_pull_node_to_ui();
   }
   void DKAttackDataWidget::commitTo(dovah::loaded_forms::components::attack_data& dst, dovah::loaded_forms::Form& dst_form) {
      dst.clear(dst_form);

      dst.race.set(dst_form, this->_subwidgets.race.picker->formStub());
      {
         using flag = dovah::loaded_forms::components::attack_data::flag;

         size_t size = this->_model->rowCount();
         for (size_t i = 0; i < size; ++i) {
            const auto* src_item = this->_model->item(i);
            if (!src_item)
               break;
            auto& dst_item = dst.attacks.emplace_back();
            dst_item.event = src_item->event_name.toStdString();
            dst_item.damage_mult   = src_item->damage_mult;
            dst_item.attack_chance = src_item->attack_chance;
            dst_item.attack_spell.set(dst_form, src_item->spell);
            dst_item.flags = 0;
            if (src_item->flags.bash)
               dst_item.flags |= flag::bash_attack;
            if (src_item->flags.ignore_weapon)
               dst_item.flags |= flag::ignore_weapon;
            if (src_item->flags.lefthanded)
               dst_item.flags |= flag::left_attack;
            if (src_item->flags.power)
               dst_item.flags |= flag::power_attack;
            if (src_item->flags.rotating)
               dst_item.flags |= flag::rotating_attack;
            if (src_item->flags.overridden)
               dst_item.flags |= flag::override_data;
            dst_item.attack_angle = src_item->angles.direction;
            dst_item.strike_angle = src_item->angles.range;
            dst_item.stagger = src_item->stagger;
            dst_item.keyword.set(dst_form, src_item->keyword);
            dst_item.recovery_time = src_item->recovery_time;
            dst_item.stamina_mult = src_item->stamina_cost_mult;
         }
      }
   }

   void DKAttackDataWidget::_pull_node_to_ui() {
      const DKAttackDataModelNode* node = nullptr;
      {
         auto* sel_model = this->_subwidgets.event_listing.view->selectionModel();
         if (sel_model) {
            auto rows = sel_model->selectedRows();
            if (!rows.isEmpty()) {
               node = this->_model->item(rows[0].row());
            }
         }
      }

      auto& ui = this->_subwidgets.event_edit;

      const auto blockers = std::array{
         QSignalBlocker(ui.name),
         QSignalBlocker(ui.damage_mult),
         QSignalBlocker(ui.attack_chance),
         QSignalBlocker(ui.stagger),
         QSignalBlocker(ui.recovery_time),
         QSignalBlocker(ui.stamina_cost_mult),
         QSignalBlocker(ui.attack_spell),
         QSignalBlocker(ui.attack_type),
         QSignalBlocker(ui.attack_angle),
         QSignalBlocker(ui.angle_range),
         QSignalBlocker(ui.knockdown),
         QSignalBlocker(ui.flags.power),
         QSignalBlocker(ui.flags.ignore_weapon),
         QSignalBlocker(ui.flags.bash),
         QSignalBlocker(ui.flags.lefthanded),
         QSignalBlocker(ui.flags.rotating),
      };
      {  // Enable states
         bool enable = node != nullptr;
         ui.name->setEnabled(enable);
         ui.damage_mult->setEnabled(enable);
         ui.attack_chance->setEnabled(enable);
         ui.stagger->setEnabled(enable);
         ui.recovery_time->setEnabled(enable);
         ui.stamina_cost_mult->setEnabled(enable);
         ui.attack_spell->setEnabled(enable);
         ui.attack_type->setEnabled(enable);
         ui.attack_angle->setEnabled(enable);
         ui.angle_range->setEnabled(enable);
         ui.knockdown->setEnabled(enable);
         ui.flags.power->setEnabled(enable);
         ui.flags.ignore_weapon->setEnabled(enable);
         ui.flags.bash->setEnabled(enable);
         ui.flags.lefthanded->setEnabled(enable);
         ui.flags.rotating->setEnabled(enable);
      }
      if (!node) {
         ui.name->setText("");
         ui.flags.power->setChecked(false);
         ui.flags.ignore_weapon->setChecked(false);
         ui.flags.bash->setChecked(false);
         ui.flags.lefthanded->setChecked(false);
         ui.flags.rotating->setChecked(false);
         return;
      }
      ui.name->setText(node->event_name);
      ui.damage_mult->setValue(node->damage_mult);
      ui.attack_chance->setValue(node->attack_chance);
      ui.stagger->setValue(node->stagger);
      ui.recovery_time->setValue(node->recovery_time);
      ui.stamina_cost_mult->setValue(node->stamina_cost_mult);
      ui.attack_spell->setFormStub(node->spell);
      ui.attack_type->setFormStub(node->keyword);
      ui.attack_angle->setValue(node->angles.direction);
      ui.angle_range->setValue(node->angles.range);
      ui.knockdown->setValue(node->knockdown);
      ui.flags.power->setChecked(node->flags.power);
      ui.flags.ignore_weapon->setChecked(node->flags.ignore_weapon);
      ui.flags.bash->setChecked(node->flags.bash);
      ui.flags.lefthanded->setChecked(node->flags.lefthanded);
      ui.flags.rotating->setChecked(node->flags.rotating);
   }
   void DKAttackDataWidget::_push_node_from_ui() {
      const DKAttackDataModelNode* node = nullptr;
      size_t row;
      {
         auto* sel_model = this->_subwidgets.event_listing.view->selectionModel();
         if (sel_model) {
            auto rows = sel_model->selectedRows();
            if (!rows.isEmpty()) {
               row  = rows[0].row();
               node = this->_model->item(row);
            }
         }
      }
      if (!node)
         return;

      auto& ui  = this->_subwidgets.event_edit;
      auto  dst = *node;

      dst.event_name = ui.name->text();
      dst.damage_mult = ui.damage_mult->value();
      dst.attack_chance = ui.attack_chance->value();
      dst.stagger = ui.stagger->value();
      dst.recovery_time = ui.recovery_time->value();
      dst.stamina_cost_mult = ui.stamina_cost_mult->value();
      dst.spell = ui.attack_spell->formStub();
      dst.keyword = ui.attack_type->formStub();
      dst.angles.direction = ui.attack_angle->value();
      dst.angles.range = ui.angle_range->value();
      dst.knockdown = ui.knockdown->value();
      dst.flags.power = ui.flags.power->isChecked();
      dst.flags.ignore_weapon = ui.flags.ignore_weapon->isChecked();
      dst.flags.bash = ui.flags.bash->isChecked();
      dst.flags.lefthanded = ui.flags.lefthanded->isChecked();
      dst.flags.rotating = ui.flags.rotating->isChecked();

      this->_model->overwrite(row, dst);
   }
#endif