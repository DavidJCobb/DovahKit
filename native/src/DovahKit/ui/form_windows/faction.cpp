#include "./faction.h"
#include <limits>
#include <optional>
#include <QContextMenuEvent>
#include "dovah/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "editor/form_stub_meta_type.h" // for QVariant::fromValue on a form stub

#include "ui/models/forms/FactionMembersModel.h"

#pragma region Models
   #pragma region Interfaction relationships
      FormDialogFaction::RelationshipModel::RelationshipModel(QObject* parent) : DKGenericListModel(parent) {
         auto& editor = DovahKitCore::get();
         QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* form) {
            if (form->form_type != dovah::form_type::faction)
               return;
            this->onFactionChanged(*form);
         });
         QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* form, bool just_being_flagged) {
            if (form->form_type != dovah::form_type::faction)
               return;
            this->onFactionDeletionImminent(*form);
         });
      }

      QVariant FormDialogFaction::RelationshipModel::headerData(int section, Qt::Orientation orientation, int role) const {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole)
            return {};
         switch (section) {
            case 0: return tr("Other faction");
            case 1: return tr("Mod");
            case 2: return tr("Reaction");
         }
         return {};
      }

      QVariant FormDialogFaction::RelationshipModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (column) {
            case 0:
               return node.faction_string;
            case 1:
               return node.mod;
            case 2:
               switch (node.reaction) {
                  using enum FactionReaction;
                  case Ally:
                     return tr("Ally");
                  case Friend:
                     return tr("Friend");
                  case Neutral:
                     return tr("Neutral");
                  case Enemy:
                     return tr("Enemy");
               }
         }
         return {};
      }
      Qt::ItemFlags FormDialogFaction::RelationshipModel::flags_of(const node_type& node, size_t column) const {
         return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
      }

      void FormDialogFaction::RelationshipModel::importFrom(const list_type& src) {
         this->performReset([this, &src]() {
            for (const auto& item : src) {
               auto* stub = item.other.get_form_stub();
               if (!stub || stub->form_type != dovah::form_type::faction)
                  continue;

               auto* node = new node_type;
               this->_nodes.push_back(node);
               node->other = stub;
               node->mod   = item.mod;
               switch (item.combat) {
                  using enum loaded_form_type::alliance_status;
                  using enum FactionReaction;
                  case ally:    node->reaction = Ally;    break;
                  case friend_: node->reaction = Friend;  break;
                  case enemy:   node->reaction = Enemy;   break;
                  case neutral: node->reaction = Neutral; break;
               }

               node->faction_string = QString::fromStdString(stub->editorID);
            }
         });
      }
      void FormDialogFaction::RelationshipModel::commitTo(loaded_form_type& dst_owner, list_type& dst) const {
         const auto& src  = this->_nodes;
         size_t      size = src.size();
         if (dst.size() < size)
            dst.resize(size);
         size_t i = 0;
         for (; i < size; ++i) {
            dst[i].other.set(dst_owner, src[i]->other);
            dst[i].mod = src[i]->mod;
            switch (src[i]->reaction) {
               using enum loaded_form_type::alliance_status;
               using enum FactionReaction;
               case Ally:    dst[i].combat = ally;    break;
               case Friend:  dst[i].combat = friend_; break;
               case Enemy:   dst[i].combat = enemy;   break;
               case Neutral: dst[i].combat = neutral; break;
               default:
                  dst[i].combat = neutral;
                  break;
            }
         }
         //
         // Delete excess elements, if any were removed:
         //
         auto s = dst.size();
         if (s != size) {
            for (; i < s; ++i)
               dst[i].other.set(dst_owner, nullptr); // ensure we maintain use info properly
            dst.resize(size);
         }
      }

      void FormDialogFaction::RelationshipModel::onFactionChanged(const dovah::form_stub& faction) {
         for (size_t i = 0; i < this->_nodes.size(); ++i) {
            auto& node = this->_nodes[i];
            if (node->other == &faction) {
               node->faction_string = QString::fromStdString(faction.editorID);

               auto qmi = this->index(i, 0, {});
               emit dataChanged(qmi, qmi, { Qt::DisplayRole, Qt::ToolTipRole });
               return;
            }
         }
      }
      void FormDialogFaction::RelationshipModel::onFactionDeletionImminent(const dovah::form_stub& faction) {
         for (size_t i = 0; i < this->_nodes.size(); ++i) {
            auto& node = this->_nodes[i];
            if (node->other == &faction) {
               this->deleteItems(i, 1);
               return;
            }
         }
      }

      void FormDialogFaction::RelationshipModel::coalesce() {
         size_t size = this->_nodes.size();
         for (size_t i = 0; i < size; ++i) {
            auto* a = this->_nodes[i];
            for (size_t j = i + 1; j < size; ++j) {
               auto* b = this->_nodes[j];
               if (a->other == b->other) {
                  a->mod      = b->mod;
                  a->reaction = b->reaction;
                  this->beginRemoveRows({}, j, j);
                  this->_nodes.removeAt(j);
                  this->endRemoveRows();
                  --j;
               }
            }
         }
      }

      QModelIndex FormDialogFaction::RelationshipModel::addItem() {
         size_t i = this->_nodes.size();
         this->beginInsertRows({}, i, i);
         auto* node = new node_type;
         this->_nodes.append(node);
         this->endInsertRows();
         return this->index(node);
      }
      void FormDialogFaction::RelationshipModel::setFactionData(const QModelIndex& qmi, dovah::form_stub& faction, int32_t mod, FactionReaction reaction) {
         if (!qmi.isValid())
            return;
         size_t i = qmi.row();
         if (i >= this->_nodes.size())
            return;

         auto* node = this->_nodes[i];
         node->other    = &faction;
         node->mod      = mod;
         node->reaction = reaction;
         node->faction_string = QString::fromStdString(faction.editorID);
         auto tl = this->index(i, 0, {});
         auto br = this->index(i, 2, {});
         emit dataChanged(tl, br, { Qt::DisplayRole, Qt::ToolTipRole });
      }
   #pragma endregion
   #pragma region Ranks
      void FormDialogFaction::RanksModel::importFrom(const list_type& src) {
         this->performReset([this, &src]() {
            for (const auto& item : src) {
               auto* node = new node_type;
               this->_nodes.push_back(node);
               *node = item;
            }
         });
      }
      void FormDialogFaction::RanksModel::commitTo(loaded_form_type& dst_owner, list_type& dst) const {
         const auto& src  = this->_nodes;
         size_t      size = src.size();
         dst.resize(size);
         for (size_t i = 0; i < size; ++i)
            dst[i] = *src[i];
      }

      QModelIndex FormDialogFaction::RanksModel::addItem() {
         std::optional<uint32_t> next_id;
         if (this->_nodes.empty()) {
            next_id = 0;
         } else {
            for (auto* node : this->_nodes) {
               if (node->id < std::numeric_limits<uint32_t>::max()) {
                  auto plus_one = node->id + 1;
                  if (!next_id.has_value() || next_id.value() < plus_one)
                     next_id = plus_one;
               }
            }
         }
         if (next_id.has_value()) {
            this->beginInsertRows({}, this->_nodes.size(), this->_nodes.size());
            auto* node = new node_type;
            this->_nodes.append(node);
            node->id = next_id.value();
            this->endInsertRows();
            return this->index(node);
         }
         return {};
      }

      QVariant FormDialogFaction::RanksModel::headerData(int section, Qt::Orientation orientation, int role) const {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole)
            return {};
         switch (section) {
            case 0: return tr("Rank");
            case 1: return tr("Masculine Title");
            case 2: return tr("Feminine Title");
            case 3: return tr("Insignia");
         }
         return {};
      }

      QVariant FormDialogFaction::RanksModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (column) {
            case 0:
               return node.id;
            case 1:
               return node.title_masc.c_str();
            case 2:
               return node.title_fem.c_str();
            case 3:
               return QString::fromStdString(node.insignia);
         }
         return {};
      }
      Qt::ItemFlags FormDialogFaction::RanksModel::flags_of(const node_type& node, size_t column) const {
         return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
      }

      void FormDialogFaction::RanksModel::setRankData(size_t row, const node_type& src) {
         size_t size = this->_nodes.size();
         if (row >= size)
            return;
         *(this->_nodes[row]) = src;


         auto tl = this->index(row, 0, {});
         auto br = this->index(row, 3, {});
         emit dataChanged(tl, br, { Qt::DisplayRole });
      }
   #pragma endregion
#pragma endregion

FormDialogFaction::FormDialogFaction(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->models.members       = new FactionMembersModel(this);
   this->models.ranks         = new RanksModel(this);
   this->models.relationships = new RelationshipModel(this);

   {  // Interfaction relation editor
      auto* view = this->ui.interfactionRelationsView;
      view->setModel(this->models.relationships);

      if (auto* header = view->horizontalHeader()) {
         header->setStretchLastSection(true);
      }
      if (auto* header = view->verticalHeader()) {
         header->setSectionResizeMode(QHeaderView::ResizeToContents);
         header->setVisible(false);
      }
      view->setSelectionBehavior(QAbstractItemView::SelectRows);
      view->setSelectionMode(QAbstractItemView::SingleSelection);
      view->setCornerButtonEnabled(false);

      auto* model     = this->models.relationships;
      auto* sel_model = this->ui.interfactionRelationsView->selectionModel();
      assert(sel_model != nullptr); // Qt itself asserts this, so I feel comfortable doing so as well.

      {
         auto* menu = this->context_menus.relationships = new QMenu(view);

         auto* action_add    = new QAction(tr("Add new"), menu);
         auto* action_remove = new QAction(tr("Remove"), menu);
         menu->addAction(action_add);
         menu->addAction(action_remove);

         view->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
         view->installEventFilter(this);

         QObject::connect(action_add, &QAction::triggered, this, [this, model, sel_model]() {
            auto qmi = model->addItem();
            if (qmi.isValid())
               sel_model->setCurrentIndex(qmi, QItemSelectionModel::SelectionFlag::ClearAndSelect);
         });
         QObject::connect(action_remove, &QAction::triggered, this, [this, model, sel_model]() {
            auto qmi = sel_model->currentIndex();
            if (!qmi.isValid())
               return;
            model->deleteItems(qmi.row(), 1);
         });

         QObject::connect(menu, &QMenu::aboutToShow, this, [sel_model, action_remove]() {
            auto qmi = sel_model->currentIndex();
            action_remove->setVisible(qmi.isValid());
         });
         QObject::connect(view, &QWidget::customContextMenuRequested, this, [this, menu, view](const QPoint& pos) {
            menu->exec(view->mapToGlobal(pos));
         });
      }

      auto* alliance_group = this->subwidgets.faction_alliance_status = new QButtonGroup(this);
      alliance_group->addButton(this->ui.interfactionEditStatusAlly,    (int)FactionReaction::Ally);
      alliance_group->addButton(this->ui.interfactionEditStatusFriend,  (int)FactionReaction::Friend);
      alliance_group->addButton(this->ui.interfactionEditStatusNeutral, (int)FactionReaction::Neutral);
      alliance_group->addButton(this->ui.interfactionEditStatusEnemy,   (int)FactionReaction::Enemy);
      for (auto* button : alliance_group->buttons()) {
         button->setProperty("value", alliance_group->id(button));
      }

      QObject::connect(sel_model, &QItemSelectionModel::currentChanged, this, [this, model](const QModelIndex& current, const QModelIndex& previous) {
         auto* alliance_group = this->subwidgets.faction_alliance_status;

         const auto blockers = std::array{
            QSignalBlocker(this->ui.interfactionEditFaction),
            QSignalBlocker(alliance_group),
         };

         const auto* data = model->node(current);
         bool enable = data != nullptr;

         this->ui.interfactionEditFaction->setEnabled(enable);
         for (auto* button : alliance_group->buttons()) {
            button->setEnabled(enable);
         }
         if (!data) {
            return;
         }

         this->ui.interfactionEditFaction->setFormStub(data->other);
         if (auto* button = alliance_group->button((int)data->reaction)) {
            button->setChecked(true);
         } else {
            this->ui.interfactionEditStatusNeutral->setChecked(true);
         }
      });
      
      this->ui.interfactionEditFaction->setAllowedFormType(dovah::form_type::faction);
      this->ui.interfactionEditFaction->setAllowNone(false);
      QObject::connect(this->ui.interfactionEditFaction, &DKFormPicker::formChanged, this, [this, sel_model, model](dovah::form_stub* stub) {
         if (!stub)
            return;
         auto        qmi  = sel_model->currentIndex();
         const auto* data = model->node(qmi);
         if (!data)
            return;
         model->setFactionData(
            qmi,
            *stub,
            data->mod,
            data->reaction
         );
      });
      QObject::connect(alliance_group, &QButtonGroup::idToggled, this, [this, sel_model, model](int id, bool checked) {
         if (!checked)
            return;

         auto        qmi  = sel_model->currentIndex();
         const auto* data = model->node(qmi);
         if (!data)
            return;
         model->setFactionData(
            qmi,
            *data->other,
            data->mod,
            (FactionReaction) id
         );
      });
   }
   {  // Member list
      auto* view = this->ui.membersView;
      view->setModel(this->models.members);

      if (auto* header = view->horizontalHeader()) {
         header->setStretchLastSection(true);
      }
      if (auto* header = view->verticalHeader()) {
         header->setSectionResizeMode(QHeaderView::ResizeToContents);
         header->setVisible(false);
      }
      view->setSelectionBehavior(QAbstractItemView::SelectRows);
      view->setSelectionMode(QAbstractItemView::SingleSelection);
      view->setCornerButtonEnabled(false);
   }
   {  // Ranks editor
      this->ui.rankEditInsignia->setStandardConfiguration(DKGameFilePicker::StandardConfiguration::Textures);
      this->ui.rankEditInsignia->setPathFormat(DKGameFilePicker::PathFormat::OmitPathStem);
      {
         auto* view = this->ui.ranksView;
         view->setModel(this->models.ranks);

         if (auto* header = view->horizontalHeader()) {
            header->setStretchLastSection(false);
         }
         if (auto* header = view->verticalHeader()) {
            header->setSectionResizeMode(QHeaderView::ResizeToContents);
            header->setVisible(false);
         }
         view->setSelectionBehavior(QAbstractItemView::SelectRows);
         view->setSelectionMode(QAbstractItemView::SingleSelection);
         view->setCornerButtonEnabled(false);

         auto& editor    = DovahKitCore::get();
         auto* model     = this->models.ranks;
         auto* sel_model = view->selectionModel();
         assert(sel_model != nullptr); // Qt itself asserts this, so I feel comfortable doing so as well.

         {
            auto* menu = this->context_menus.ranks = new QMenu(this);

            auto* action_add    = new QAction(tr("Add new"), menu);
            auto* action_remove = new QAction(tr("Remove"), menu);
            menu->addAction(action_add);
            menu->addAction(action_remove);

            view->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
            view->installEventFilter(this);

            QObject::connect(action_add, &QAction::triggered, this, [this, model, sel_model]() {
               auto qmi = model->addItem();
               if (qmi.isValid())
                  sel_model->setCurrentIndex(qmi, QItemSelectionModel::SelectionFlag::ClearAndSelect);
            });
            QObject::connect(action_remove, &QAction::triggered, this, [this, model, sel_model]() {
               auto qmi = sel_model->currentIndex();
               if (!qmi.isValid())
                  return;
               model->deleteItems(qmi.row(), 1);
            });

            QObject::connect(menu, &QMenu::aboutToShow, this, [sel_model, action_remove]() {
               auto qmi = sel_model->currentIndex();
               action_remove->setVisible(qmi.isValid());
            });
            QObject::connect(view, &QWidget::customContextMenuRequested, this, [this, menu, view](const QPoint& pos) {
               menu->exec(view->mapToGlobal(pos));
            });
         }

         QObject::connect(sel_model, &QItemSelectionModel::currentChanged, this, [this, model, &editor](const QModelIndex& current, const QModelIndex& previous) {
            const auto blockers = std::array{
               QSignalBlocker(this->ui.rankEditID),
               QSignalBlocker(this->ui.rankEditInsignia),
               QSignalBlocker(this->ui.rankEditNameF),
               QSignalBlocker(this->ui.rankEditNameM),
            };

            const auto* data = model->node(current);
            bool enable = data != nullptr;

            this->ui.rankEditID->setEnabled(enable);
            this->ui.rankEditInsignia->setEnabled(enable);
            this->ui.rankEditNameF->setEnabled(enable);
            this->ui.rankEditNameM->setEnabled(enable);
            if (!data) {
               return;
            }

            this->ui.rankEditID->setValue(data->id);
            this->ui.rankEditInsignia->setPath(QString::fromStdString(data->insignia));
            this->ui.rankEditNameF->setText(editor.convert_localized_string(data->title_fem));
            this->ui.rankEditNameM->setText(editor.convert_localized_string(data->title_masc));
            this->ui.rankEditInsigniaPreview->setAsset(this->ui.rankEditInsignia->rawPath()); // use rawPath to add any needed path prefixes
         });
         QObject::connect(this->ui.rankEditID, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, model, sel_model](int value) {
            auto  index = sel_model->currentIndex();
            auto* src   = model->node(index);
            if (!src)
               return;
            auto data = *src;
            data.id = value;
            model->setRankData(index.row(), data);
         });
         QObject::connect(this->ui.rankEditNameF, &QLineEdit::textChanged, this, [this, &editor, model, sel_model](QString text) {
            auto  index = sel_model->currentIndex();
            auto* src   = model->node(index);
            if (!src)
               return;
            auto data = *src;
            editor.assign_localized_string(data.title_fem, this->ui.rankEditNameF->text());
            model->setRankData(index.row(), data);
         });
         QObject::connect(this->ui.rankEditNameM, &QLineEdit::textChanged, this, [this, &editor, model, sel_model](QString text) {
            auto  index = sel_model->currentIndex();
            auto* src   = model->node(index);
            if (!src)
               return;
            auto data = *src;
            editor.assign_localized_string(data.title_masc, this->ui.rankEditNameM->text());
            model->setRankData(index.row(), data);
         });
         QObject::connect(this->ui.rankEditInsignia, &DKGameFilePicker::pathChanged, this, [this, model, sel_model](QString path) {
            auto  index = sel_model->currentIndex();
            auto* src   = model->node(index);
            if (!src)
               return;
            auto data = *src;
            data.insignia = path.toStdString();
            model->setRankData(index.row(), data);
            this->ui.rankEditInsigniaPreview->setAsset(this->ui.rankEditInsignia->rawPath()); // use rawPath to add any needed path prefixes
         });
      }
   }
   {  // Crime tab
      this->ui.crimeJailOutfit->setAllowedFormType(dovah::form_type::outfit);
      this->ui.crimeSharedCrimeFactionList->setAllowedFormType(dovah::form_type::formlist);

      ui::set_range<uint16_t>(this->ui.crimeGoldMurder);
      ui::set_range<uint16_t>(this->ui.crimeGoldAssault);
      ui::set_unsigned_range<float>(this->ui.crimeGoldTheftMult);
      ui::set_range<uint16_t>(this->ui.crimeGoldPickpocket);
      ui::set_range<uint16_t>(this->ui.crimeGoldTrespass);
      ui::set_range<uint16_t>(this->ui.crimeGoldWerewolf);
      ui::set_range<uint16_t>(this->ui.crimeGoldJailEscape);
   }
   {  // Vendor tab
      this->ui.vendorHourStart->setRange(0, 24);
      this->ui.vendorHourEnd->setRange(0, 24);
      this->ui.vendorLocRadius->setRange(0, 65535);
      this->ui.vendorWaresList->setAllowedFormType(dovah::form_type::formlist);

      {
         auto* widget = this->ui.vendorWaresInclExcl;
         widget->clear();
         widget->addItem(tr("Whitelist", "vendor list invert"), false);
         widget->addItem(tr("Blacklist", "vendor list invert"), true);
      }
   }

   this->load(); // this creates the working copy.
}
void FormDialogFaction::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(editor.convert_localized_string(working.name));

   {  // General tab
      ui::bind(this->ui.flagHiddenFromPlayer, working.faction_flags, loaded_form_type::faction_flag::hidden_from_player);
      ui::bind(this->ui.flagSpecialCombat,    working.faction_flags, loaded_form_type::faction_flag::special_combat);
      ui::bind(this->ui.flagCanBeOwner,       working.faction_flags, loaded_form_type::faction_flag::can_be_owner);

      this->models.relationships->importFrom(working.relationships); // Interfaction relations

      if (auto* stub = this->formStub()) {
         this->models.members->setFaction(*stub);
      }
   }
   {  // Ranks tab
      this->models.ranks->importFrom(working.ranks);
   }
   {  // Crime tab
      ui::bind(this->ui.groupboxTrackCrime, working.faction_flags, loaded_form_type::faction_flag::track_crime);

      ui::bind(this->ui.flagIgnoreNonMemberVictimMurder,     working.faction_flags, loaded_form_type::faction_flag::ignore_murder);
      ui::bind(this->ui.flagIgnoreNonMemberVictimAssault,    working.faction_flags, loaded_form_type::faction_flag::ignore_assault);
      ui::bind(this->ui.flagIgnoreNonMemberVictimPickpocket, working.faction_flags, loaded_form_type::faction_flag::ignore_pickpocketing);
      ui::bind(this->ui.flagIgnoreNonMemberVictimTheft,      working.faction_flags, loaded_form_type::faction_flag::ignore_theft);
      ui::bind(this->ui.flagIgnoreNonMemberVictimTrespass,   working.faction_flags, loaded_form_type::faction_flag::ignore_trespass);
      ui::bind(this->ui.flagIgnoreNonMemberVictimWerewolf,   working.faction_flags, loaded_form_type::faction_flag::ignore_werewolf_transformation);
      //
      ui::bind(this->ui.flagMembersDontReportCrimes, working.faction_flags, loaded_form_type::faction_flag::dont_report_crimes_against_members);

      ui::bind(this->ui.crimeExteriorJailMarker,       working.prison_marker, working);
      ui::bind(this->ui.crimeFollowerWaitMarker,       working.follower_wait_marker, working);
      ui::bind(this->ui.crimeStolenGoodsContainer,     working.evidence_chest, working);
      ui::bind(this->ui.crimePlayerInventoryContainer, working.player_belongings_chest, working);
      ui::bind(this->ui.crimeJailOutfit,               working.jail_outfit, working);

      ui::bind(this->ui.crimeSharedCrimeFactionList, working.crime_group, working);
      ui::bind(this->ui.flagAttackOnSight, working.crime_values.attack_on_sight);
      ui::bind(this->ui.flagArrest,        working.crime_values.arrest);

      ui::bind(this->ui.flagUseDefaultCrimeGold, working.faction_flags, loaded_form_type::faction_flag::crime_gold_is_default);
      ui::bind(this->ui.crimeGoldMurder,     working.crime_values.murder);
      ui::bind(this->ui.crimeGoldAssault,    working.crime_values.assault);
      ui::bind(this->ui.crimeGoldTheftMult,  working.crime_values.theft_multiplier);
      ui::bind(this->ui.crimeGoldPickpocket, working.crime_values.pickpocket);
      ui::bind(this->ui.crimeGoldTrespass,   working.crime_values.trespass);
      ui::bind(this->ui.crimeGoldWerewolf,   working.crime_values.werewolf_transformation);
      ui::bind(this->ui.crimeGoldJailEscape, working.crime_values.jail_escape);
   }
   {  // Vendor tab
      ui::bind(this->ui.vendorGroupbox, working.faction_flags, loaded_form_type::faction_flag::vendor);

      ui::bind(this->ui.vendorHourStart, working.vendor_data.start_hour);
      ui::bind(this->ui.vendorHourEnd,   working.vendor_data.end_hour);
      ui::bind(this->ui.vendorLocRadius,         working.vendor_data.radius);
      {
         auto* widget = this->ui.vendorWaresInclExcl;
         widget->setCurrentIndex(widget->findData(this->form->vendor_data.vendor_list_is_blacklist));
         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
            bool invert = this->ui.vendorWaresInclExcl->currentData().toBool();
            this->form->vendor_data.vendor_list_is_blacklist = invert;
         });
      }
      ui::bind(this->ui.vendorWaresList, working.vendor_list, working);

      ui::bind(this->ui.vendorChest, working.vendor_chest, working);
      ui::bind(this->ui.flagOnlyBuysStolenItems, working.vendor_data.buys_stolen);

      this->ui.vendorConditions->importFrom(working, working.vendor_conditions);
   }
}
void FormDialogFaction::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   editor.assign_localized_string(working.name, this->ui.name->text());

   this->models.relationships->coalesce();
   this->models.relationships->commitTo(working, working.relationships);
   this->models.ranks->commitTo(working, working.ranks);
   std::sort(working.ranks.begin(), working.ranks.end(), [](const auto& a, const auto& b) {
      return a.id < b.id;
   });

   this->ui.vendorConditions->exportTo(working, working.vendor_conditions);
}