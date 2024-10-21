#include "./SceneActorParticipationModel.h"
#include "./SceneFormVisualEditor_impl/Actor.h"

void SceneActorParticipationModel::addActor(const SceneFormVisualEditor_impl::Actor& src) {
   constexpr auto _copy_data = [](auto& dst, const auto& src) {
      dst.name = src.cached.alias_name;
      dst.flags.no_player_activation = src.participation_flags.no_player_activation;
      dst.flags.optional = src.participation_flags.optional;
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
void SceneActorParticipationModel::commitActor(SceneFormVisualEditor_impl::Actor& dst) const {
   for (auto& item : this->_data) {
      if (item.alias_id != dst.alias_id)
         continue;
      dst.participation_flags.no_player_activation = item.flags.no_player_activation;
      dst.participation_flags.optional = item.flags.optional;
      return;
   }
}
      
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex SceneActorParticipationModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex SceneActorParticipationModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex SceneActorParticipationModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int SceneActorParticipationModel::rowCount(const QModelIndex& parent) const /*override*/ {
         if (parent.isValid())
            return 0;
         return this->_data.size();
      }
      /*virtual*/ int SceneActorParticipationModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant SceneActorParticipationModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         int i = index.row();
         int c = index.column();
         if (i >= this->_data.size())
            return {};
         if (c >= ColumnCount)
            return {};
         if (role != Qt::CheckStateRole)
            return {};
         auto& item    = this->_data[i];
         bool  checked = false;
         switch (c) {
            case Column::NoPlayerDialogue:
               checked = item.flags.no_player_activation;
               break;
            case Column::Optional:
               checked = item.flags.optional;
               break;
         }
         return checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
      }
      /*virtual*/ Qt::ItemFlags SceneActorParticipationModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         auto flags = Qt::ItemFlag::ItemIsUserCheckable | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemNeverHasChildren | Qt::ItemFlag::ItemIsEnabled;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant SceneActorParticipationModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (role == Qt::WhatsThisRole) {
         if (orientation == Qt::Orientation::Horizontal) {
            switch (section) {
               case Column::NoPlayerDialogue:
                  return tr("If checked, the actor will not respond when the player tries to activate them; the \"This actor is busy\" message will be displayed instead.");
               case Column::Optional:
                  return tr("If checked, the scene will not wait for this actor to become available before starting, and the actor's scene actions will be considered completed instantly.");
            }
         }
         return {};
      }

      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};

      if (orientation == Qt::Orientation::Horizontal) {
         switch (section) {
            case Column::NoPlayerDialogue:
               return tr("No Player Activation");
            case Column::Optional:
               return tr("Optional");
         }
         return {};
      }

      if (section < 0 || section >= this->_data.size())
         return {};
      return this->_data[section].name;
   }
#pragma endregion