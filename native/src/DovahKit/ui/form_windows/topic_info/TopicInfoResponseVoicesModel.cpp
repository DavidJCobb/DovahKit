#include "./TopicInfoResponseVoicesModel.h"
#include <QColor>
#include "helpers/vectors/re_sort_item_within.h"
#include "dovah/form_stub.h"
#include "dovah/utils/compute_voice_file_location.h"
#include "editor/core.h"
#include "editor/subsystems/assets.h"
#include "ui/types/game_file_path.h"

void TopicInfoResponseVoicesModel::node_type::set_audio_file_path(const std::string& path) {
   if (this->audio_file.path == path)
      return;
   this->audio_file.path = QString::fromStdString(path);
   this->audio_file.scoped_path = ui::types::game_file_path(this->audio_file.path);
   this->audio_file.scoped_path.scope_to_stem_folder("sound");

   auto& assets = dovahkit::subsystems::assets::get();
   // this will be cleaner once we have a backend type for asset paths:
   this->audio_file.exists = assets.game_asset_exists(this->audio_file.scoped_path.to_string().toStdString(), true);
}

TopicInfoResponseVoicesModel::TopicInfoResponseVoicesModel(QObject* parent) : QAbstractItemModel(parent) {
   // ensure this exists, so we can use the fast getter from here on out:
   dovahkit::subsystems::assets::get_or_create();

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, [this]() {
      this->beginResetModel();
      this->_data.clear();
      this->_voice_file_location_info = {};
      this->endResetModel();
   });
   QObject::connect(&editor, &DovahKitCore::formCreated,          this, [this](dovah::form_stub* stub) {
      if (stub->form_type != dovah::form_type::voicetype)
         return;
      this->_insert_voicetype(*stub, true);
   });
   QObject::connect(&editor, &DovahKitCore::formModified,         this, [this](dovah::form_stub* stub) {
      if (stub->form_type != dovah::form_type::voicetype)
         return;
      for(size_t i = 0; i < this->_data.size(); ++i) {
         auto& node = this->_data[i];
         if (node.voicetype == stub) {
            node.cached.voicetype_editor_id = QString::fromStdString(stub->editorID);

            auto qmi = this->index(i, Column::Voicetype, {});
            emit this->dataChanged(qmi, qmi);
            this->_re_sort_item(i);
            return;
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      if (stub->form_type != dovah::form_type::voicetype)
         return;
      size_t size = this->_data.size();
      for (size_t i = 0; i < size; ++i) {
         auto& node = this->_data[i];
         if (node.voicetype == stub) {
            this->beginRemoveRows({}, i, i);
            this->_data.erase(this->_data.begin() + i);
            --i;
            --size;
            this->endRemoveRows();
            continue;
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::dataSaveComplete,     this, &TopicInfoResponseVoicesModel::_on_active_file_saved);
   if (editor.has_data())
      this->_pull_all_voicetypes();
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex TopicInfoResponseVoicesModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= column_count)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex TopicInfoResponseVoicesModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex TopicInfoResponseVoicesModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int TopicInfoResponseVoicesModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int TopicInfoResponseVoicesModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return column_count;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant TopicInfoResponseVoicesModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_data.size())
            return {};
         auto& node = this->_data[index.row()];
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (index.column()) {
                  case Column::Voicetype:
                     return node.cached.voicetype_editor_id;
                  case Column::FilePath:
                     return node.audio_file.path;
               }
               return {};
            case Qt::ItemDataRole::ForegroundRole:
               if (index.column() == Column::FilePath) {
                  if (!node.audio_file.exists)
                     return QColor(128, 128, 128);
               }
               return {};
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags TopicInfoResponseVoicesModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
    /*virtual*/ QVariant TopicInfoResponseVoicesModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
       if (orientation != Qt::Orientation::Horizontal)
          return {};
       if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
          return {};
       switch (section) {
          using enum Column::enumeration;
          case Voicetype:
             return tr("Voicetype");
          case FilePath:
             return tr("File Path");
       }
       return {};
    }
#pragma endregion

void TopicInfoResponseVoicesModel::setVoiceFileLocationInfo(VoiceFileLocationInfo&& src) {
   auto& assets = dovahkit::subsystems::assets::get_or_create();

   this->_voice_file_location_info = std::move(src);
   for (auto& node : this->_data) {
      auto path = dovah::compute_voice_file_location(
         this->_voice_file_location_info.data_filename,
         node.voicetype->get_editor_id(),
         this->_voice_file_location_info.quest_editor_id,
         this->_voice_file_location_info.topic_editor_id,
         this->_voice_file_location_info.info_form_id,
         this->_voice_file_location_info.response_uid,
         "fuz"
      );
      node.set_audio_file_path(path);
   }
   
   auto tl = this->index(0,                       Column::FilePath, {});
   auto br = this->index(this->_data.size() - 1, Column::FilePath, {});
   emit dataChanged(tl, br);
}

/*static*/ bool TopicInfoResponseVoicesModel::_compare_for_sort(const node_type& a, const node_type& b) {
   return a.cached.voicetype_editor_id.compare(b.cached.voicetype_editor_id) < 0;
}
decltype(TopicInfoResponseVoicesModel::_data)::iterator TopicInfoResponseVoicesModel::_insertion_point_for(const node_type& item) {
   return std::upper_bound(
      this->_data.begin(),
      this->_data.end(),
      item,
      &_compare_for_sort
   );
}
void TopicInfoResponseVoicesModel::_re_sort_item(size_t from) {
   auto& list = this->_data;
   if (from >= list.size())
      return;
   
   bool moved = false;
   cobb::vectors::re_sort_item_within(
      list,
      list.begin() + from,
      &_compare_for_sort,
      [&moved, this, &list](decltype(_data)::iterator from_it, decltype(_data)::iterator to_it) {
         size_t from  = std::distance(list.begin(), from_it);
         size_t to    = std::distance(list.begin(), to_it);
         moved = this->beginMoveRows(
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

void TopicInfoResponseVoicesModel::_insert_voicetype(dovah::form_stub& voicetype, bool emit_signals) {
   node_type node = {
      .voicetype = &voicetype,
      .cached = {
         .voicetype_editor_id = QString::fromStdString(voicetype.editorID)
      },
   };
   {
      auto path = dovah::compute_voice_file_location(
         this->_voice_file_location_info.data_filename,
         voicetype.get_editor_id(),
         this->_voice_file_location_info.quest_editor_id,
         this->_voice_file_location_info.topic_editor_id,
         this->_voice_file_location_info.info_form_id,
         this->_voice_file_location_info.response_uid,
         "fuz"
      );
      node.set_audio_file_path(path);
   }
   auto it = this->_insertion_point_for(node);
   if (emit_signals) {
      auto i = std::distance(this->_data.begin(), it);
      this->beginInsertRows({}, i, i);
   }
   this->_data.insert(this->_insertion_point_for(node), std::move(node));
   if (emit_signals) {
      this->endInsertRows();
   }
}
void TopicInfoResponseVoicesModel::_on_active_file_saved() {
   auto& editor = DovahKitCore::get();
   //
   // Handle the possibility of the active file's name changing.
   //
   auto filename = editor.get_active_file_name();
   for (size_t i = 0; i < this->_data.size(); ++i) {
      auto& node = this->_data[i];
      if (editor.is_form_defined_in_active_file(node.voicetype)) {
         auto path = dovah::compute_voice_file_location(
            this->_voice_file_location_info.data_filename,
            node.voicetype->get_editor_id(),
            this->_voice_file_location_info.quest_editor_id,
            this->_voice_file_location_info.topic_editor_id,
            this->_voice_file_location_info.info_form_id,
            this->_voice_file_location_info.response_uid,
            "fuz"
         );
         node.set_audio_file_path(path);

         auto qmi = this->index(i, Column::FilePath, {});
         emit this->dataChanged(qmi, qmi);
      }
   }
}
void TopicInfoResponseVoicesModel::_pull_all_voicetypes() {
   auto& assets = dovahkit::subsystems::assets::get_or_create();
   auto& editor = DovahKitCore::get();

   this->beginResetModel();
   this->_data.clear();
   editor.for_each_form_of_type(dovah::form_type::voicetype, [this, &assets](dovah::form_stub* stub) -> bool {
      this->_insert_voicetype(*stub, false);
      return false;
   });
   this->endResetModel();
}