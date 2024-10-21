#include "./SceneActorBehaviorModel.h"
#include "./SceneFormVisualEditor_impl/Actor.h"

SceneActorBehaviorModel::SceneActorBehaviorModel(QObject* parent) : QAbstractItemModel(parent) {
}

void SceneActorBehaviorModel::addActor(const SceneFormVisualEditor_impl::Actor& src) {
   constexpr auto _copy_flags = [](auto& dst, const auto& src) {
      dst.pause = src.pause;
      dst.end   = src.end;
   };
   constexpr auto _copy_data = [](auto& dst, const auto& src) {
      dst.name = src.cached.alias_name;
      _copy_flags(dst.behavior_flags.death, src.behavior_flags.death);
      _copy_flags(dst.behavior_flags.combat, src.behavior_flags.combat);
      _copy_flags(dst.behavior_flags.dialogue, src.behavior_flags.dialogue);
      _copy_flags(dst.behavior_flags.observe_corpse, src.behavior_flags.observe_corpse);
   };

   size_t size = this->_data.size();
   for (size_t i = 0; i < size; ++i) {
      auto& item = this->_data[i];
      if (item.alias_id == src.alias_id) {
         _copy_data(item, src);

         auto tl = this->index(i, 0, {});
         auto br = this->index(i, 7, {});
         emit dataChanged(tl, br);
         return;
      }
   }
   this->beginInsertRows({}, size, size);
   auto& item = this->_data.emplace_back();
   item.alias_id = src.alias_id;
   _copy_data(item, src);
   this->endInsertRows();
}
void SceneActorBehaviorModel::commitActor(SceneFormVisualEditor_impl::Actor& dst) const {
   constexpr auto _copy_flags = [](auto& dst, const auto& src) {
      dst.pause = src.pause;
      dst.end   = src.end;
   };

   for (auto& item : this->_data) {
      if (item.alias_id != dst.alias_id)
         continue;
      _copy_flags(dst.behavior_flags.death, item.behavior_flags.death);
      _copy_flags(dst.behavior_flags.combat, item.behavior_flags.combat);
      _copy_flags(dst.behavior_flags.dialogue, item.behavior_flags.dialogue);
      _copy_flags(dst.behavior_flags.observe_corpse, item.behavior_flags.observe_corpse);
      return;
   }
}
      
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex SceneActorBehaviorModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= 8)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex SceneActorBehaviorModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex SceneActorBehaviorModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int SceneActorBehaviorModel::rowCount(const QModelIndex& parent) const /*override*/ {
         if (parent.isValid())
            return 0;
         return this->_data.size();
      }
      /*virtual*/ int SceneActorBehaviorModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return 8;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant SceneActorBehaviorModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         int i = index.row();
         int c = index.column();
         if (i >= this->_data.size())
            return {};
         if (c >= 8)
            return {};
         if (role != Qt::CheckStateRole)
            return {};
         auto& item    = this->_data[i];
         bool  checked = false;
         switch (c) {
            case 0:
               checked = item.behavior_flags.death.pause;
               break;
            case 1:
               checked = item.behavior_flags.death.end;
               break;
            case 2:
               checked = item.behavior_flags.combat.pause;
               break;
            case 3:
               checked = item.behavior_flags.combat.end;
               break;
            case 4:
               checked = item.behavior_flags.dialogue.pause;
               break;
            case 5:
               checked = item.behavior_flags.dialogue.end;
               break;
            case 6:
               checked = item.behavior_flags.observe_corpse.pause;
               break;
            case 7:
               checked = item.behavior_flags.observe_corpse.end;
               break;
         }
         return checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
      }
      /*virtual*/ Qt::ItemFlags SceneActorBehaviorModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         auto flags = Qt::ItemFlag::ItemIsUserCheckable | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemNeverHasChildren;
         if (index.column() != 0) // Death, Pause
            flags |= Qt::ItemFlag::ItemIsEnabled;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant SceneActorBehaviorModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};

      if (orientation == Qt::Orientation::Horizontal) {
         switch (section) {
            case 0:
               return tr("Death: Pause");
            case 1:
               return tr("Death: End");
            case 2:
               return tr("Combat: Pause");
            case 3:
               return tr("Combat: End");
            case 4:
               return tr("Dialogue: Pause");
            case 5:
               return tr("Dialogue: End");
            case 6:
               return tr("Observe Corpse: Pause");
            case 7:
               return tr("Observe Corpse: End");
         }
         return {};
      }

      if (section < 0 || section >= this->_data.size())
         return {};
      return this->_data[section].name;
   }
#pragma endregion