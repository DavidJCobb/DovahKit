#include "./QuestDialogueBranchlessTopicsModel.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/form_stub_meta_type.h"
#include "editor/subsystems/game_settings/core.h"
#include "editor/core.h"

QuestDialogueBranchlessTopicsModel::QuestDialogueBranchlessTopicsModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &QuestDialogueBranchlessTopicsModel::_update_all_topic_subtype_names);
   if (editor.has_data())
      this->_update_all_topic_subtype_names();

   auto& gss = dovahkit::subsystems::game_settings::core::get();
   QObject::connect(&gss, &std::decay_t<decltype(gss)>::settingValueChanged, this, &QuestDialogueBranchlessTopicsModel::_update_topic_subtype_name);
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex QuestDialogueBranchlessTopicsModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex QuestDialogueBranchlessTopicsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex QuestDialogueBranchlessTopicsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int QuestDialogueBranchlessTopicsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int QuestDialogueBranchlessTopicsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant QuestDialogueBranchlessTopicsModel::data(const QModelIndex& index, int role) const /*override*/ {
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
            case Column::Subtype:
               return _subtype_name(item.cached.subtype);
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags QuestDialogueBranchlessTopicsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
      /*virtual*/ QVariant QuestDialogueBranchlessTopicsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case Column::EditorID:
               return tr("Editor ID");
            case Column::FormID:
               return tr("Form ID");
            case Column::Subtype:
               return tr("Subtype");
            case Column::Priority:
               return tr("Priority");
            case Column::DisplayText:
               return tr("Display Text");
         }
         return {};
      }
#pragma endregion
      
void QuestDialogueBranchlessTopicsModel::setCategory(dovah::dialogue::category c) {
   if (this->_category == c)
      return;
   this->_category = c;
   if (!this->_datastore)
      return;

   this->beginResetModel();
   {
      auto& src_list = this->_datastore->all_branchless_topics();
      auto& dst_list = this->_data;
      dst_list.clear();
      
      for (auto* item : dst_list) {
         if (item->cached.category != c)
            continue;
         dst_list.push_back(item);
      }
   }
   this->endResetModel();
}
void QuestDialogueBranchlessTopicsModel::setDatastore(datastore_type* ds) {
   if (ds == this->_datastore)
      return;
   if (this->_datastore)
      QObject::disconnect(this->_datastore, nullptr, this, nullptr);
   this->_datastore = ds;
   if (ds) {
      QObject::connect(ds, &datastore_type::on_filled,  this, &QuestDialogueBranchlessTopicsModel::_fill);
      QObject::connect(ds, &QObject::destroyed,         this, &QuestDialogueBranchlessTopicsModel::_fill);
      QObject::connect(ds, &datastore_type::on_cleared, this, &QuestDialogueBranchlessTopicsModel::_fill);
      QObject::connect(ds, &datastore_type::on_topic_added, this, [this](const node_type& node) {
         auto&  list = this->_data;
         size_t i    = list.size();
         this->beginInsertRows({}, i, i);
         list.push_back(&node);
         this->endInsertRows();

         this->_re_sort_node(node);
      });
      QObject::connect(ds, &datastore_type::on_topic_edited, this, &QuestDialogueBranchlessTopicsModel::_on_node_edited);
      QObject::connect(ds, &datastore_type::on_topic_removed, this, [this](const node_type& node) {
         if (node.parent)
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

void QuestDialogueBranchlessTopicsModel::_fill() {
   this->beginResetModel();
   this->_data.clear();
   if (this->_datastore) {
      auto& src = this->_datastore->all_branchless_topics();
      for (auto* item : src) {
         if (item->cached.category != this->_category)
            continue;
         this->_data.push_back(item);
      }
   }
   this->endResetModel();
}
void QuestDialogueBranchlessTopicsModel::_on_node_edited(const node_type& node) {
   if (node.parent)
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
void QuestDialogueBranchlessTopicsModel::_re_sort_node(const node_type& item) {
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
   }
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
   list.erase(entry_it);
   list.insert(list.begin() + to, &item);
   this->endMoveRows();
}
decltype(QuestDialogueBranchlessTopicsModel::_data)::iterator QuestDialogueBranchlessTopicsModel::_insertion_point_for(const node_type& item) {
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
         return a->cached.editor_id.compare(b->cached.editor_id, Qt::CaseInsensitive) < 0;
      }
   );
}

QString QuestDialogueBranchlessTopicsModel::_subtype_name(uint32_t subtype_signature) const {
   auto& list = dovah::dialogue::all_topic_subtypes;
   for (size_t i = 0; i < list.size(); ++i) {
      if (list[i].signature == subtype_signature) {
         return this->_subtype_names[i];
      }
   }
   return "";
}

void QuestDialogueBranchlessTopicsModel::_update_topic_subtype_name(const char* game_setting_name) {
   constexpr const std::string_view gmst_prefix = "sTopicSubtypeText";

   std::string_view gs_name = game_setting_name;
   std::string_view gs_subtype;
   {
      if (!gs_name.starts_with(gmst_prefix))
         return;

      auto c_name      = dovah::dialogue::internal_name_for_category(this->_category);
      auto gs_category = gs_name.substr(gmst_prefix.size());
      if (!gs_category.starts_with(c_name))
         return;
      gs_subtype = gs_category.substr(gs_category.size());
   }

   auto& gss     = dovahkit::subsystems::game_settings::core::get();
   auto  variant = gss.get_setting_value(game_setting_name);
   
   QString value;
   if (std::holds_alternative<dovah::localized_string>(variant)) {
      value = DovahKitCore::get().convert_localized_string(std::get<dovah::localized_string>(variant));
   }
   if (value.isEmpty()) {
      variant = gss.get_setting_default_value(game_setting_name);
      if (std::holds_alternative<dovah::localized_string>(variant)) {
         value = DovahKitCore::get().convert_localized_string(std::get<dovah::localized_string>(variant));
      }
   }

   auto&  list = dovah::dialogue::all_topic_subtypes;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto& dfn = list[i];
      if (dfn.category != this->_category)
         continue;
      if (gs_subtype != dfn.internal_name)
         continue;

      if (value.isEmpty())
         value = QString::fromStdString(std::string(dfn.internal_name));
      this->_subtype_names[i] = value;
      break;
   }
}
void QuestDialogueBranchlessTopicsModel::_update_all_topic_subtype_names() {
   auto& gss = dovahkit::subsystems::game_settings::core::get();
   auto& src = dovah::dialogue::all_topic_subtypes;
   auto& dst = this->_subtype_names;
   for (size_t i = 0; i < dst.size(); ++i) {
      auto gs_name = src[i].game_setting_for_name();
      auto variant = gss.get_setting_value(gs_name.c_str());
      
      QString value;
      if (std::holds_alternative<dovah::localized_string>(variant)) {
         value = DovahKitCore::get().convert_localized_string(std::get<dovah::localized_string>(variant));
      }
      if (value.isEmpty()) {
         variant = gss.get_setting_default_value(gs_name.c_str());
         if (std::holds_alternative<dovah::localized_string>(variant)) {
            value = DovahKitCore::get().convert_localized_string(std::get<dovah::localized_string>(variant));
         }
         if (value.isEmpty()) {
            value = QString::fromStdString(std::string(src[i].internal_name));
         }
      }
      dst[i] = value;
   }
}