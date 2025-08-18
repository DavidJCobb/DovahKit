#include "./MusicTrackCuePointsModel.h"

MusicTrackCuePointsModel::MusicTrackCuePointsModel(QObject* parent) : QAbstractItemModel(parent) {
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex MusicTrackCuePointsModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex MusicTrackCuePointsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex MusicTrackCuePointsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int MusicTrackCuePointsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int MusicTrackCuePointsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant MusicTrackCuePointsModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_data.size())
            return {};
         auto& src = this->_data[index.row()];
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
            case Qt::UserRole:
               return src;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags MusicTrackCuePointsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
      #pragma region Write-access
         /*virtual*/ bool MusicTrackCuePointsModel::setData(const QModelIndex& index, const QVariant& value, int role) /*override*/ {
            if (!index.isValid() || index.row() >= this->_data.size() || index.column() >= ColumnCount)
               return false;
            if (role != Qt::UserRole)
               return false;

            this->_data[index.row()] = value.toFloat();
            emit dataChanged(index, index);
            this->_re_sort_item(index.row());
            return true;
         }
      #pragma endregion
   #pragma endregion
   /*virtual*/ QVariant MusicTrackCuePointsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      return tr("Time in seconds");
   }
#pragma endregion

void MusicTrackCuePointsModel::importData(const std::vector<float>& src_list) {
   this->beginResetModel();
   this->_data = src_list;
   this->endResetModel();
}
void MusicTrackCuePointsModel::exportData(std::vector<float>& list) const {
   list = this->_data;
}

QModelIndex MusicTrackCuePointsModel::append() {
   size_t i = this->_data.size();
   this->beginInsertRows({}, i, i);
   auto& added = this->_data.emplace_back();
   if (i > 0) {
      added = this->_data[i - 1];
   }
   this->endInsertRows();
   return this->index(i, 0, {});
}

decltype(MusicTrackCuePointsModel::_data)::iterator MusicTrackCuePointsModel::_insertion_point_for(float item) {
   return std::upper_bound(
      this->_data.begin(),
      this->_data.end(),
      item,
      [](const float a, const float b) -> bool {
         return a < b;
      }
   );
}
void MusicTrackCuePointsModel::_re_sort_item(size_t from) {
   auto& list = this->_data;
   if (from >= list.size())
      return;
   auto   item = list[from];
   size_t to;
   bool   moving_upward_in_list;
   {
      auto dst_it = this->_insertion_point_for(item);
      //
      // Can't use the iterator directly because we'll be doing a removal first, which will 
      // invalidate it.
      //
      to = std::distance(list.begin(), dst_it);
      moving_upward_in_list = to < from;
   }
   if (to == from)
      return;
   this->beginMoveRows(
      {},
      from, // first to move
      from, // last  to move
      {},
      to
   );
   if (!moving_upward_in_list) {
      //
      // We move `entry` by first removing it from the list, and then inserting it into the 
      // list at the desired index. If we're moving `entry` downward within the list, then 
      // its removal will displace the intended destination by -1.
      //
      --to;
   }
   list.erase(list.begin() + from);
   list.insert(list.begin() + to, item);
   this->endMoveRows();
}