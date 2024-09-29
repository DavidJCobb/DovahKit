#include "./QuestDialogueBranchedTopicsModel.h"
#include "helpers/vectors/re_sort_item_within.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/form_stub_meta_type.h"

QuestDialogueBranchedTopicsModel::QuestDialogueBranchedTopicsModel(QObject* parent) : QAbstractItemModel(parent) {
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex QuestDialogueBranchedTopicsModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex QuestDialogueBranchedTopicsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex QuestDialogueBranchedTopicsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int QuestDialogueBranchedTopicsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int QuestDialogueBranchedTopicsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant QuestDialogueBranchedTopicsModel::data(const QModelIndex& index, int role) const /*override*/ {
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
            case Column::DisplayText:
               return item.cached.display_text;
            case Column::Priority:
               return item.cached.priority;
            case Column::IsBranchStartingTopic:
               if (this->_root) {
                  if (this->_root->starting_topic == &item) {
                     return tr("<", "branch starting topic indicator");
                  }
               }
               return {};
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags QuestDialogueBranchedTopicsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
      /*virtual*/ QVariant QuestDialogueBranchedTopicsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case Column::EditorID:
               return tr("Editor ID");
            case Column::FormID:
               return tr("Form ID");
            case Column::IsBranchStartingTopic:
               return tr("Starting");
            case Column::Priority:
               return tr("Priority");
            case Column::DisplayText:
               return tr("Display Text");
         }
         return {};
      }
#pragma endregion

void QuestDialogueBranchedTopicsModel::setDatastore(datastore_type* ds) {
   if (ds == this->_datastore)
      return;
   if (this->_datastore) {
      QObject::disconnect(this->_datastore, nullptr, this, nullptr);
   }
   this->_datastore = ds;
   if (!ds) {
      this->_root = nullptr;
   }
   if (ds) {
      QObject::connect(ds, &datastore_type::on_filled, this, &QuestDialogueBranchedTopicsModel::_fill);

      QObject::connect(ds, &QObject::destroyed, this, [this]() {
         this->_root = nullptr;
         this->_fill();
      });
      QObject::connect(ds, &datastore_type::on_cleared, this, [this]() {
         this->_root = nullptr;
         this->_fill();
      });
      QObject::connect(ds, &datastore_type::on_branch_removed, this, [this](const container_type& node) {
         if (this->_root != &node)
            return;
         this->_root = nullptr;
         this->_fill();
      });
      QObject::connect(ds, &datastore_type::on_topic_added, this, [this](const node_type& node) {
         if (!this->_root || node.parent != this->_root)
            return;

         auto&  list = this->_data;
         size_t i    = list.size();
         this->beginInsertRows({}, i, i);
         list.push_back(&node);
         this->endInsertRows();

         this->_re_sort_node(node);
      });
      QObject::connect(ds, &datastore_type::on_topic_edited, this, &QuestDialogueBranchedTopicsModel::_on_node_edited);
      QObject::connect(ds, &datastore_type::on_topic_removed, this, [this](const node_type& node) {
         if (!this->_root || node.parent != this->_root)
            return;

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
void QuestDialogueBranchedTopicsModel::setRootBranch(dovah::form_stub* stub) {
   if (!this->_datastore)
      return;
   if (!stub) {
      if (!this->_root)
         return;
      this->beginResetModel();
      this->_root = nullptr;
      this->_data.clear();
      this->endResetModel();
      return;
   }
   if (auto* prior = this->_root) {
      if (prior->stub == stub)
         return;
   }
   const container_type* item = this->_datastore->item_for_branch_stub(*stub);
   
   this->beginResetModel();
   this->_root = item;
   this->_data.clear();
   if (item) {
      this->_data.reserve(item->topics.size());
      for (auto* info : item->topics) {
         this->_data.push_back(info);
      }
   }
   this->endResetModel();
}

const QuestDialogueBranchedTopicsModel::node_type* QuestDialogueBranchedTopicsModel::node(size_t row) const {
   if (row >= this->_data.size())
      return nullptr;
   return this->_data[row];
}
size_t QuestDialogueBranchedTopicsModel::index_of(const dovah::form_stub& stub) const {
   auto&  list = this->_data;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i)
      if (list[i]->stub == &stub)
         return i;
   return (size_t)-1;
}

void QuestDialogueBranchedTopicsModel::_fill() {
   this->beginResetModel();
   this->_data.clear();
   if (this->_root) {
      for (auto* item : this->_root->topics)
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
void QuestDialogueBranchedTopicsModel::_on_node_edited(const node_type& node) {
   if (!this->_root || node.parent != this->_root)
      return;

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
void QuestDialogueBranchedTopicsModel::_re_sort_node(const node_type& item) {
   auto& list = this->_data;
            
   auto entry_it = std::find(list.begin(), list.end(), &item);
   if (entry_it == list.end())
      return;

   bool moved = false;
   cobb::vectors::re_sort_item_within(
      list,
      entry_it,
      [](const node_type* a, const node_type* b) -> bool {
         if (!a->stub && b->stub)
            return true;
         if (!b->stub && a->stub)
            return false;
         return a->cached.editor_id.localeAwareCompare(b->cached.editor_id) < 0;
      },
      [&moved, this, &list](decltype(_data)::iterator from_it, decltype(_data)::iterator to_it) {
         moved = true;
         size_t from  = std::distance(list.begin(), from_it);
         size_t to    = std::distance(list.begin(), to_it);

         this->beginMoveRows(
            {},
            from, // first to move
            from, // last  to move
            {},
            (to < from) ? to : to + 1 // Qt API design jank
         );
      }
   );
   if (moved)
      this->endMoveRows();
}
decltype(QuestDialogueBranchedTopicsModel::_data)::iterator QuestDialogueBranchedTopicsModel::_insertion_point_for(const node_type& item) {
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