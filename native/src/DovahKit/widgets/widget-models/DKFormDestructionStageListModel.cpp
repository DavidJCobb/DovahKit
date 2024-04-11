#include "./DKFormDestructionStageListModel.h"
#include "editor/form_stub_meta_type.h"

QVariant DKFormDestructionStageListModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   switch (role) {
      case Qt::TextAlignmentRole:
         switch (column) {
            case Column::HealthPercentage:
            case Column::SelfDPS:
               return (int)(Qt::AlignRight | Qt::AlignVCenter);
         }
         return {};

      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         switch (column) {
            case Column::HealthPercentage:
               return node.health_percent;
            case Column::SelfDPS:
               return node.self_damage_rate;
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
Qt::ItemFlags DKFormDestructionStageListModel::flags_of(const node_type&, size_t column) const {
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

/*virtual*/ QVariant DKFormDestructionStageListModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
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

QModelIndex DKFormDestructionStageListModel::appendStage(const node_type& src) {
   auto at = this->_nodes.size();

   this->beginInsertRows({}, at, at);
   this->_nodes.push_back(new node_type{ src });
   this->endInsertRows();

   return this->index(at, 0, {});
}

void DKFormDestructionStageListModel::replaceStages(const std::vector<node_type>& items) {
   this->beginResetModel();

   this->_nodes.clear();
   for (auto& item : items) {
      auto* node = new node_type{item};
      this->_nodes.push_back(node);
   }

   this->endResetModel();
}

void DKFormDestructionStageListModel::setStage(int row, const node_type& src) {
   if (row < 0 || row >= this->_nodes.size())
      return;
   auto* node = this->_nodes[row];
   if (!node)
      return;
   *node = src;
   this->emitNodeChanged(*node);
}

const DKFormDestructionStageListModel::node_type* DKFormDestructionStageListModel::stage(int row) const {
   if (row < 0 || row >= this->_nodes.size())
      return nullptr;
   return this->_nodes[row];
}