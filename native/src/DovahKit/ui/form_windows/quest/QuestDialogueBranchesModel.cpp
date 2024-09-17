#include "./QuestDialogueBranchesModel.h"
#include "helpers/vectors/move_item_within.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/form_stub_meta_type.h"

QuestDialogueBranchesModel::QuestDialogueBranchesModel(QObject* parent) : QAbstractItemModel(parent) {
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex QuestDialogueBranchesModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex QuestDialogueBranchesModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex QuestDialogueBranchesModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int QuestDialogueBranchesModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int QuestDialogueBranchesModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant QuestDialogueBranchesModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_data.size())
            return {};
         if (role == FormStubRole) {
            return QVariant::fromValue(this->_data[index.row()]->stub);
         }
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         const auto& item = *this->_data[index.row()];
         switch (index.column()) {
            case Column::EditorID:
               return item.cached.editor_id;
            case Column::FormID:
               return editor_helpers::form_id_to_string(item.stub->formID);
            case Column::Flags:
               {
                  QString flags;
                  switch (item.cached.type) {
                     case node_type::Type::TopLevel:
                        flags += "T";
                        break;
                     case node_type::Type::Blocking:
                        flags += "B";
                        break;
                     default:
                        flags += " ";
                  }
                  if (item.cached.exclusive) {
                     flags += "E";
                  }
                  return flags;
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags QuestDialogueBranchesModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
      /*virtual*/ QVariant QuestDialogueBranchesModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case Column::EditorID:
               return tr("Editor ID");
            case Column::FormID:
               return tr("Form ID");
            case Column::Flags:
               return tr("Flags");
         }
         return {};
      }
#pragma endregion

void QuestDialogueBranchesModel::setDatastore(datastore_type* ds) {
   if (ds == this->_datastore)
      return;
   if (this->_datastore)
      QObject::disconnect(this->_datastore, nullptr, this, nullptr);
   this->_datastore = ds;
   if (ds) {
      QObject::connect(ds, &datastore_type::on_filled,  this, &QuestDialogueBranchesModel::_fill);
      QObject::connect(ds, &QObject::destroyed,         this, &QuestDialogueBranchesModel::_fill);
      QObject::connect(ds, &datastore_type::on_cleared, this, &QuestDialogueBranchesModel::_fill);
      QObject::connect(ds, &datastore_type::on_branch_added, this, [this](const node_type& node) {
         auto&  list = this->_data;
         size_t i    = list.size();
         this->beginInsertRows({}, i, i);
         list.push_back(&node);
         this->endInsertRows();

         this->_re_sort_node(node);
      });
      QObject::connect(ds, &datastore_type::on_branch_edited, this, &QuestDialogueBranchesModel::_on_node_edited);
      QObject::connect(ds, &datastore_type::on_branch_removed, this, [this](const node_type& node) {
         auto&  list = this->_data;
         size_t size = list.size();
         for (size_t i = 0; i < size; ++i) {
            auto* item = list[i];
            if (item != &node)
               continue;

            this->beginRemoveRows({}, i, i);
            list.erase(list.begin() + i);
            this->endRemoveRows();
            return;
         }
      });
   }
   this->_fill();
}

const QuestDialogueBranchesModel::node_type* QuestDialogueBranchesModel::node(size_t row) const {
   if (row >= this->_data.size())
      return nullptr;
   return this->_data[row];
}
size_t QuestDialogueBranchesModel::index_of(const dovah::form_stub& stub) const {
   auto&  list = this->_data;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i)
      if (list[i]->stub == &stub)
         return i;
   return (size_t)-1;
}

void QuestDialogueBranchesModel::_fill() {
   this->beginResetModel();
   this->_data.clear();
   if (this->_datastore) {
      auto& src = this->_datastore->all_branches();
      for (auto* item : src)
         this->_data.push_back(item);
      std::sort(
         this->_data.begin(),
         this->_data.end(),
         [](const node_type* a, const node_type* b) -> bool {
            return a->cached.editor_id.localeAwareCompare(b->cached.editor_id) < 0;
         }
      );
   }
   this->endResetModel();
}
void QuestDialogueBranchesModel::_on_node_edited(const node_type& node) {
   auto&  list = this->_data;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item != &node)
         continue;

      auto tl = this->index(i, 0, {});
      auto br = this->index(i, ColumnCount - 1, {});
      emit dataChanged(tl, br);

      this->_re_sort_node(node);
      return;
   }
}
void QuestDialogueBranchesModel::_re_sort_node(const node_type& item) {
   auto& list = this->_data;
            
   auto entry_it = std::find(list.begin(), list.end(), &item);
   if (entry_it == list.end())
      return;
   size_t from = std::distance(list.begin(), entry_it);
   size_t to;
   bool   moving_upward_in_list;
   {
      auto dst_it = this->_insertion_point_for(item);
      //
      // Can't use the iterator directly because we'll be doing a removal first, which will 
      // invalidate it.
      //
      to = std::distance(list.begin(), dst_it);
      moving_upward_in_list = dst_it < entry_it;

      if (!moving_upward_in_list)
         //
         // NOTE: `_insertion_point_for` gets the position at which we'd INSERT a NEW node, 
         //       but in this case, we're instead MOVING a node. The node itself is "in the 
         //       way" and needs to be accounted for.
         //
         --to;

      if (to == from)
         return;
   }
   this->beginMoveRows(
      {},
      from, // first to move
      from, // last  to move
      {},
      moving_upward_in_list ? to : to + 1 // Qt API design jank
   );
   bool moved = cobb::vectors::move_item_within<false>(list, from, (int)to - (int)from);
   assert(moved);
   this->endMoveRows();
}
decltype(QuestDialogueBranchesModel::_data)::iterator QuestDialogueBranchesModel::_insertion_point_for(const node_type& item) {
   if (!item.stub)
      //
      // We may want to prepend an "ALL" or "ORPHANED" entry to the top of the list.
      // This is a hook for that.
      //
      return this->_data.begin();

   return std::upper_bound(
      this->_data.begin(),
      this->_data.end(),
      &item,
      [](const node_type* a, const node_type* b) -> bool {
         if (!a->stub && b->stub)
            return true;
         if (!b->stub && a->stub)
            return false;
         return a->cached.editor_id.localeAwareCompare(b->cached.editor_id) < 0;
      }
   );
}