#include "./DKFormDestructionStageListModel.h"
#include "editor/form_stub_meta_type.h"

QVariant DKFormDestructionStageListModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
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

QModelIndex DKFormDestructionStageListModel::insertStage(const node_type& src) {
   auto it = _insertion_point_for(src.health_percent, src.damage_stage);
   auto i = std::distance(this->_nodes.begin(), it);
   this->beginInsertRows({}, i, i);
   this->_nodes.insert(it, new node_type{ src });
   this->endInsertRows();

   return this->index(i, 0, {});
}

void DKFormDestructionStageListModel::replaceStages(const std::vector<node_type>& items) {
   this->beginResetModel();

   this->_nodes.clear();
   for (auto& item : items) {
      auto* node = new node_type{item};
      this->_nodes.push_back(node);
   }
   std::stable_sort(this->_nodes.begin(), this->_nodes.end(), _sort_nodes);

   this->endResetModel();
}

QModelIndex DKFormDestructionStageListModel::setStage(int row, const node_type& src) {
   if (row < 0 || row >= this->_nodes.size())
      return {};
   auto* node = this->_nodes[row];
   if (!node)
      return {};

   auto prior_health_perc = node->health_percent;
   auto after_health_perc = src.health_percent;

   size_t to;
   if (prior_health_perc != after_health_perc) {
      //
      // Do this FIRST. The `_insertion_point_for` function checks the list as it currently 
      // exists; we can't properly find where to move our node to if we change it before we 
      // go looking.
      //
      to = std::distance(this->_nodes.begin(), _insertion_point_for(src.health_percent, src.damage_stage));
   }

   *node = src;
   this->emitNodeChanged(*node);

   //
   // Sort stages in order of descending health.
   //
   if (prior_health_perc != after_health_perc) {
      if (row != to) {
         bool moving_down = to > row;
         if (!this->beginMoveRows({}, row, row, {}, to + (moving_down ? 1 : 0)))
            assert(false && "can't move the rows?!");
         this->_nodes.move(row, to - (moving_down ? 1 : 0));
         this->endMoveRows();

         row = to;
      }
   }

   return this->index(row, 0, {});
}

const DKFormDestructionStageListModel::node_type* DKFormDestructionStageListModel::stage(int row) const {
   if (row < 0 || row >= this->_nodes.size())
      return nullptr;
   return this->_nodes[row];
}

decltype(DKFormDestructionStageListModel::_nodes)::iterator DKFormDestructionStageListModel::_insertion_point_for(unsigned int health_percentage, unsigned int damage_stage) {
   return std::upper_bound(
      this->_nodes.begin(),
      this->_nodes.end(),
      std::pair{ health_percentage, damage_stage },
      [](auto pair, const node_type* a) {
         if (pair.first > a->health_percent)
            return true;
         if (a->health_percent == pair.first && pair.second < a->damage_stage)
            return true;
         return false;
      }
   );
}

// Return true if `a` should be sorted before `b`.
/*static*/ bool DKFormDestructionStageListModel::_sort_nodes(const node_type* a, const node_type* b) {
   if (a->health_percent > b->health_percent)
      return true;
   if (a->damage_stage < b->damage_stage)
      return true;
   return false;
}