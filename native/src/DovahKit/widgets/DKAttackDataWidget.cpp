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
#endif

DKAttackDataWidget::DKAttackDataWidget(QWidget* parent) : QWidget(parent) {
   auto& ui = this->_subwidgets;

   #if !defined(QT_DESIGNER_LIB)
      this->_model = new DKAttackDataModel(this);
   #endif

   #pragma region Create and configure widgets and layout
   {
      auto* layout = new QVBoxLayout(this);

      layout->addWidget(ui.race.container = new QWidget(this));
      layout->addWidget(ui.event_listing.container = new QWidget(this));
      layout->addWidget(ui.event_edit.container = new QWidget(this));

      {  // Attack Race
         auto& ui     = this->_subwidgets.race;
         auto* layout = new QHBoxLayout(ui.container);

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

         auto* view = ui.view = new QTableView(this);
         view->setModel(this->_model);
         {
            view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
            view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
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
            #if !defined(QT_DESIGNER_LIB)
               header->setColumnFlex(DKAttackDataModel::Column::Name,    1, 0, metrics.boundingRect("Event Name Event Name").width() * 1.5F + 4);
               header->setColumnFlex(DKAttackDataModel::Column::Spell,   1, 0, metrics.boundingRect("ReallyCoolFireball01").width() * 1.5F + 4);
               header->setColumnFlex(DKAttackDataModel::Column::Keyword, 1, 0, metrics.boundingRect("CoolSpellKeyword").width() * 1.5F + 4);
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
         layout->addWidget(view);

         auto* buttons_container = new QWidget(this);
         layout->addWidget(buttons_container);
         auto* buttons_layout = new QVBoxLayout(buttons_container);

         buttons_layout->addWidget(ui.button_create = new QPushButton(tr("Add"),    buttons_container));
         buttons_layout->addWidget(ui.button_delete = new QPushButton(tr("Remove"), buttons_container));
         buttons_layout->addStretch(1);
      }

      {  // Event editing
         auto& ui     = this->_subwidgets.event_edit;
         auto* layout = new QGridLayout(ui.container);

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
            layout->addWidget(label,  row, col);
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
            layout->addWidget(label,  row, col);
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
            auto* label  = new QLabel(tr("Attack Angle:"), this);
            auto* widget = ui.attack_angle = new QDoubleSpinBox(this);
            label->setBuddy(widget);
            widget->setRange(-360, 360);
            widget->setWhatsThis(tr("The direction of the attack relative to the attacking actor. A value of 180 degrees, for example, would cause the attack to travel backwards from the actor."));
            //
            layout->addWidget(label,  row, col);
            layout->addWidget(widget, row, col + 1);
         }
         ++row;
         {  // Angle Range
            auto* label  = new QLabel(tr("Angle Range:"), this);
            auto* widget = ui.angle_range = new QDoubleSpinBox(this);
            label->setBuddy(widget);
            widget->setRange(-360, 360);
            widget->setWhatsThis(tr("The angle tolerance on the attack. For example, a value of 35 means that targets within ±35 degrees of the Attack Angle would be hit by the attack."));
            //
            layout->addWidget(label,  row, col);
            layout->addWidget(widget, row, col + 1);
         }
         ++row;
         {  // Knockdown
            auto* label  = new QLabel(tr("Knockdown:"), this);
            auto* widget = ui.knockdown = new QDoubleSpinBox(this);
            label->setBuddy(widget);
            widget->setRange(-360, 360);
            //
            layout->addWidget(label,  row, col);
            layout->addWidget(widget, row, col + 1);
         }
         ++row;
         {
            auto& flags = ui.flags;
            {
               auto* widget = flags.power = new QCheckBox(tr("Power Attack"), this);
               widget->setWhatsThis(tr("If this is checked, then the attack is a Power Attack: the actor can only perform it if they have enough Stamina, and it will use that amount of Stamina."));
               layout->addWidget(widget,  row, col, 1, 2);
            }
            ++row;
            {
               auto* widget = flags.ignore_weapon = new QCheckBox(tr("Ignore Weapon"), this);
               widget->setWhatsThis(tr("If checked, this attack will ignore the damage formula for the actor's weapon."));
               layout->addWidget(widget,  row, col, 1, 2);
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
   {
      auto* view      = this->_subwidgets.event_listing.view;
      auto* sel_model = view->selectionModel();
      auto* model     = this->_model;
      static_assert(false, "TODO");
   }
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
      static_assert(false, "TODO");
   }
   void DKAttackDataWidget::commitTo(dovah::loaded_forms::components::attack_data& dst, dovah::loaded_forms::Form& dst_form) {
      static_assert(false, "TODO");
   }
#endif