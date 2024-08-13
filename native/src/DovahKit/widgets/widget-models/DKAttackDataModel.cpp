#include "./DKAttackDataModel.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

void DKAttackDataModelNode::recache_editor_ids() {
   if (auto* stub = this->keyword)
      this->cached.keywordEditorID = QString::fromStdString(stub->editorID);
   else
      this->cached.keywordEditorID = "";

   if (auto* stub = this->spell)
      this->cached.spellEditorID = QString::fromStdString(stub->editorID);
   else
      this->cached.spellEditorID = "";
}

DKAttackDataModel::DKAttackDataModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &DKAttackDataModel::clear);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      for(size_t i = 0; i < this->_nodes.size(); ++i) {
         auto* node = this->_nodes[i];
         if (node->keyword == stub) {
            node->cached.keywordEditorID = QString::fromStdString(stub->editorID);

            auto qmi = this->index(i, Column::Keyword, {});
            emit this->dataChanged(qmi, qmi);
         }
         if (node->spell == stub) {
            node->cached.spellEditorID = QString::fromStdString(stub->editorID);

            auto qmi = this->index(i, Column::Spell, {});
            emit this->dataChanged(qmi, qmi);
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      for (size_t i = 0; i < this->_nodes.size(); ++i) {
         auto* node = this->_nodes[i];
         if (node->keyword == stub) {
            node->keyword = nullptr;
            node->cached.keywordEditorID = "";

            auto qmi = this->index(i, Column::Keyword, {});
            emit this->dataChanged(qmi, qmi);
         }
         if (node->spell == stub) {
            node->spell = nullptr;
            node->cached.spellEditorID = "";

            auto qmi = this->index(i, Column::Spell, {});
            emit this->dataChanged(qmi, qmi);
         }
      }
   });
}

QVariant DKAttackDataModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   switch (role) {
      case Qt::TextAlignmentRole:
         switch (column) {
            case Column::AttackChance:
            case Column::DamageMult:
            case Column::Knockdown:
            case Column::RecoveryTime:
            case Column::Stagger:
            case Column::StaminaCostMult:
               return (int)(Qt::AlignRight | Qt::AlignVCenter);
         }
         return {};

      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         switch (column) {
            case Column::Name:
               return node.event_name;
            case Column::DamageMult:
               return QString::number(node.damage_mult, 'f', 2);
            case Column::AttackChance:
               return QString::number(node.attack_chance, 'f', 2);
            case Column::Stagger:
               return QString::number(node.stagger, 'f', 2);
            case Column::RecoveryTime:
               return QString::number(node.recovery_time, 'f', 2);
            case Column::StaminaCostMult:
               return QString::number(node.stamina_cost_mult, 'f', 2);
            case Column::Spell:
               return node.cached.spellEditorID;
            case Column::Keyword:
               return node.cached.keywordEditorID;
            case Column::Angle:
               return trUtf8((const char*)u8"%1±%2")
                  .arg(QString::number(node.angles.direction, 'f', 2), 6)
                  .arg(QString::number(node.angles.range, 'f', 2), 6);
            case Column::Knockdown:
               return QString::number(node.knockdown, 'f', 2);
         }
         return {};
   }
   return {};
}
Qt::ItemFlags DKAttackDataModel::flags_of(const node_type&, size_t column) const {
   auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
   return flags;
}

/*virtual*/ QVariant DKAttackDataModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (role != Qt::DisplayRole)
      return {};
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   switch (section) {
      using enum Column::enumeration;
      case Name:
         return tr("Event");
      case DamageMult:
         return tr("Damage Mult");
      case AttackChance:
         return tr("Attack Chance");
      case Stagger:
         return tr("Stagger");
      case RecoveryTime:
         return tr("Recovery Time");
      case StaminaCostMult:
         return tr("Stamina Cost Mult");
      case Spell:
         return tr("Spell");
      case Keyword:
         return tr("Keyword");
      case Angle:
         return tr("Angle");
      case Knockdown:
         return tr("Knockdown");
   }
   return {};
}

QModelIndex DKAttackDataModel::create() {
   auto i = this->_nodes.size();
   this->beginInsertRows({}, i, i);
   this->_nodes.push_back(new node_type{});
   this->endInsertRows();
   return this->index(i, 0, {});
}
QModelIndex DKAttackDataModel::overwrite(int row, const node_type& src) {
   if (row < 0 || row >= this->_nodes.size())
      return {};
   auto* node = this->_nodes[row];
   *node = src;
   node->recache_editor_ids();
   
   auto tl = this->index(row, 0, {});
   auto br = this->index(row, column_count, {});
   emit dataChanged(tl, br);
}
const DKAttackDataModel::node_type* DKAttackDataModel::item(int row) const {
   if (row < 0 || row >= this->_nodes.size())
      return nullptr;
   return this->_nodes[row];
}

void DKAttackDataModel::overwriteAllItems(const std::vector<node_type>& src) {
   this->performReset([this, &src]() {
      size_t size = src.size();
      this->_nodes.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i] = new node_type{ src[i] };
         node->recache_editor_ids();
      }
   });
}