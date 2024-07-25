#include "./DKAttackDataModel.h"

QVariant DKAttackDataModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   switch (role) {
      case Qt::TextAlignmentRole:
         switch (column) {
            case Column::HealthPercentage:
            case Column::SelfDPS:
               return (int)(Qt::AlignRight | Qt::AlignVCenter);
            case Column::FlagCapDamage:
            case Column::FlagDestroy:
            case Column::FlagDisable:
            case Column::FlagIgnoreExternal:
            case Column::ModelDamageStage:
               return (int)(Qt::AlignHCenter | Qt::AlignVCenter);
         }
         return {};

      case Qt::CheckStateRole:
         switch (column) {
            using Flag = DKFormDestructionDataButton::DestructionStageFlag;
            case Column::FlagCapDamage:
               return !!(node.flags & Flag::cap_damage) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
            case Column::FlagDestroy:
               return !!(node.flags & Flag::destroy_object) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
            case Column::FlagDisable:
               return !!(node.flags & Flag::disable_object) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
            case Column::FlagIgnoreExternal:
               return !!(node.flags & Flag::ignore_external_damage) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
         }
         return {};

      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         switch (column) {
            case Column::HealthPercentage:
               return node.health_percent;
            case Column::SelfDPS:
               return node.self_damage_rate;
            case Column::ModelDamageStage:
               return node.damage_stage;
            case Column::Debris:
               if (auto* stub = node.debris) // keep blank if NONE
                  return QVariant::fromValue(stub);
               return {};
            case Column::Explosion:
               if (auto* stub = node.explosion) // keep blank if NONE
                  return QVariant::fromValue(stub);
               return {};
            case Column::ReplacementModel:
               return QString::fromStdString(node.replacement_model.model_path);
         }
         return {};
   }
   return {};
}
Qt::ItemFlags DKAttackDataModel::flags_of(const node_type&, size_t column) const {
   auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;

   switch (column) {
      using enum Column::enumeration;
      case FlagCapDamage:
      case FlagDisable:
      case FlagDestroy:
      case FlagIgnoreExternal:
         flags |= Qt::ItemFlag::ItemIsUserCheckable;
         break;
   }

   return flags;
}

/*virtual*/ QVariant DKAttackDataModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (role != Qt::DisplayRole)
      return {};
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   switch (section) {
      using enum Column::enumeration;
      case HealthPercentage:
         return tr("Health %");
      case SelfDPS:
         return tr("Self DPS");
      case FlagCapDamage:
         return tr("Cap Damage");
      case FlagDisable:
         return tr("Disable");
      case FlagDestroy:
         return tr("Destroy");
      case FlagIgnoreExternal:
         return tr("Ignore External");
      case ModelDamageStage:
         return tr("Model Damage Stage");
      case Explosion:
         return tr("Explosion");
      case Debris:
         return tr("Debris");
      case ReplacementModel:
         return tr("Replacement Model");
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
   *this->_nodes[row] = src;
   
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
         this->_nodes[i] = new node_type{ src[i] };
      }
   });
}