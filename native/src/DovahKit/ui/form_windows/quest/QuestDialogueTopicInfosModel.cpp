#include "./QuestDialogueTopicInfosModel.h"
#include "helpers/vectors/move_item_within.h"
#include "dovah/forms/TopicInfo.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/form_stub_meta_type.h"

QuestDialogueTopicInfosModel::QuestDialogueTopicInfosModel(QObject* parent) : QAbstractItemModel(parent) {
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex QuestDialogueTopicInfosModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex QuestDialogueTopicInfosModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex QuestDialogueTopicInfosModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int QuestDialogueTopicInfosModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int QuestDialogueTopicInfosModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant QuestDialogueTopicInfosModel::data(const QModelIndex& index, int role) const /*override*/ {
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
            case Column::Conditions:
               if (role == Qt::ToolTipRole) {
                  return item.cached.conditions_tooltip;
               }
               return item.cached.conditions;
            case Column::Flags:
               {
                  using flag = dovah::loaded_forms::TopicInfo::info_flag;

                  QString out;

                  if (item.cached.links_to_any_topics)
                     out += tr("C", "info flag");

                  if (item.cached.flags & flag::random_end)
                     out += tr("E", "info flag");
                  else if (item.cached.flags & flag::random)
                     out += tr("R", "info flag");

                  if (item.cached.flags & flag::goodbye)
                     out += tr("G", "info flag");
                  if (item.cached.hours_until_reset)
                     out += tr("O(%1)", "info flag").arg(item.cached.hours_until_reset, 1, 'f', 2);
                  if (item.cached.has_own_prompt)
                     out += tr("P", "info flag");
                  if (item.cached.flags & flag::say_once)
                     out += tr("S", "info flag");
                  if (item.cached.flags & flag::walk_away_invisible_in_menu)
                     out += tr("V", "info flag");
                  if (item.cached.flags & flag::walk_away)
                     out += tr("W", "info flag");

                  return out;
               }
               break;
            case Column::HasResultScript:
               if (item.cached.has_end_fragment)
                  return tr("Y", "info has end fragment");
               return {};
            case Column::InFaction:
               return item.cached.faction;
            case Column::IsVoiceType:
               return item.cached.voicetype;
            case Column::Speaker:
               return item.cached.speaker;
            case Column::Target:
               return item.cached.target;
            case Column::InfoText:
               {
                  QString text;
                  if (role == Qt::ToolTipRole) {
                     text = item.cached.responses_tooltip;
                     if (item.deleted)
                        text.prepend("<b>DELETED</b> - ");
                     if (item.cached.uses_shared_info)
                        text.prepend("<b>&lt;&lt;Shared&gt;&gt;</b> ");
                  } else {
                     text = item.cached.responses;
                     if (item.deleted)
                        text.prepend("DELETED - ");
                     if (item.cached.uses_shared_info)
                        text.prepend("<<Shared>> ");
                  }
                  return text;
               }
            case Column::ResponseCount:
               return item.cached.response_count;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags QuestDialogueTopicInfosModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
      /*virtual*/ QVariant QuestDialogueTopicInfosModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case Column::InfoText:
               return tr("Text");
            case Column::EditorID:
               return tr("Editor ID");
            case Column::FormID:
               return tr("Form ID");
            case Column::Flags:
               return tr("Flags");
            case Column::ResponseCount:
               return tr("# Responses");
            case Column::Speaker:
               return tr("Speaker");
            case Column::Target:
               return tr("Target");
            case Column::IsVoiceType:
               return tr("Voicetype");
            case Column::InFaction:
               return tr("Faction");
            case Column::Conditions:
               return tr("Conditions");
            case Column::HasResultScript:
               return tr("Has Result Script");
         }
         return {};
      }
#pragma endregion

void QuestDialogueTopicInfosModel::setDatastore(datastore_type* ds) {
   if (ds == this->_datastore)
      return;
   if (this->_datastore)
      QObject::disconnect(this->_datastore, nullptr, this, nullptr);
   if (!ds) {
      this->_root = nullptr;
   }
   this->_datastore = ds;
   if (ds) {
      QObject::connect(ds, &datastore_type::on_filled, this, &QuestDialogueTopicInfosModel::_fill);
      QObject::connect(ds, &QObject::destroyed, this, [this]() {
         this->beginResetModel();
         this->_data.clear();
         this->_root = nullptr;
         this->endResetModel();
      });
      QObject::connect(ds, &datastore_type::on_cleared, this, [this]() {
         this->beginResetModel();
         this->_data.clear();
         this->_root = nullptr;
         this->endResetModel();
      });
      QObject::connect(ds, &datastore_type::on_branch_removed, this, [this](const datastore_type::Branch& node) {
         if (!this->_root || this->_root->parent != &node)
            return;
         this->_on_root_destroyed();
      });
      QObject::connect(ds, &datastore_type::on_topic_removed, this, [this](const container_type& node) {
         if (this->_root != &node)
            return;
         this->_on_root_destroyed();
      });
      QObject::connect(ds, &datastore_type::on_info_added, this, [this](const node_type& node) {
         if (!this->_root || node.parent != this->_root)
            return;

         size_t i = 0;
         size_t size;
         bool   found = false;
         {
            auto& list = this->_root->infos;
            size = list.size();
            assert(size == this->_data.size() + 1);
            for (; i < list.size(); ++i) {
               if (list[i] == &node) {
                  found = true;
                  break;
               }
            }
         }

         auto& list = this->_data;
         this->beginInsertRows({}, i, i);
         if (i == size) {
            list.push_back(&node);
         } else {
            list.insert(list.begin() + i, &node);
         }
         this->endInsertRows();
      });
      QObject::connect(ds, &datastore_type::on_info_edited, this, &QuestDialogueTopicInfosModel::_on_node_edited);
      QObject::connect(ds, &datastore_type::on_info_removed, this, [this](const node_type& node) {
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
      QObject::connect(ds, &datastore_type::on_info_reordered, this, &QuestDialogueTopicInfosModel::_find_node_and_handle_reordering);
   }
   this->_fill();
}
void QuestDialogueTopicInfosModel::setRootTopic(dovah::form_stub* stub) {
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
   const container_type* item = this->_datastore->item_for_topic_stub(*stub);
   
   this->beginResetModel();
   this->_root = item;
   this->_data.clear();
   if (item) {
      this->_data.reserve(item->infos.size());
      for (auto* info : item->infos) {
         this->_data.push_back(info);
      }
   }
   this->endResetModel();
}

const QuestDialogueTopicInfosModel::node_type* QuestDialogueTopicInfosModel::node(size_t row) const {
   if (row >= this->_data.size())
      return nullptr;
   return this->_data[row];
}
size_t QuestDialogueTopicInfosModel::index_of(const dovah::form_stub& stub) const {
   auto&  list = this->_data;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i)
      if (list[i]->stub == &stub)
         return i;
   return (size_t)-1;
}

void QuestDialogueTopicInfosModel::_fill() {
   this->beginResetModel();
   this->_data.clear();
   if (this->_root) {
      for (auto* item : this->_root->infos)
         this->_data.push_back(item);
   }
   this->endResetModel();
}
void QuestDialogueTopicInfosModel::_on_root_destroyed() {
   this->_root = nullptr;
   this->beginResetModel();
   this->_data.clear();
   this->endResetModel();
}
void QuestDialogueTopicInfosModel::_on_node_edited(const node_type& node) {
   if (!this->_root || node.parent != this->_root)
      return;

   size_t i = this->_find_node_and_handle_reordering(node);
   if (i == (size_t)-1)
      return;

   auto tl = this->index(i, 0, {});
   auto br = this->index(i, ColumnCount - 1, {});
   emit dataChanged(tl, br);
}
size_t QuestDialogueTopicInfosModel::_find_node_and_handle_reordering(const node_type& node) {
   if (!this->_root || node.parent != this->_root)
      return (size_t)-1;

   size_t index_prior = 0;
   size_t index_after = 0;
   {
      auto&  src_list = this->_root->infos;
      size_t src_size = src_list.size();
      bool   found    = false;
      for (size_t i = 0; i < src_size; ++i) {
         if (src_list[i] == &node) {
            index_after = i;
            found       = true;
         }
      }
      assert(found);
      if (!found)
         return (size_t)-1;
   }
   {
      auto&  dst_list = this->_data;
      size_t dst_size = dst_list.size();
      bool   found    = false;
      for (size_t i = 0; i < dst_size; ++i) {
         if (dst_list[i] == &node) {
            index_prior = i;
            found       = true;
         }
      }
      assert(found);
      if (!found)
         return (size_t)-1;
   }
   if (index_prior != index_after) {
      this->_on_node_reordered(node, index_prior, index_after);
   }
   return index_after;
}
void QuestDialogueTopicInfosModel::_on_node_reordered(const node_type& node, size_t from, size_t to) {
   auto& list = this->_data;
   assert(from < list.size());
   assert(to   < list.size());
   assert(list[from] == &node);

   bool moving_upward_in_list = to < from;
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