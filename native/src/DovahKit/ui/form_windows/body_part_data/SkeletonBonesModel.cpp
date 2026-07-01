#include "./SkeletonBonesModel.h"
#pragma region Headers for loading NIFs
   #include "dovah/files/bsa/bsa_archived_file.h"
   #include "editor/subsystems/assets.h"
   #include "nif/blocks/NiNode.h"
   #include "nif/file.h"
#pragma endregion

SkeletonBonesModel::SkeletonBonesModel(QObject* parent) : QAbstractItemModel(parent) {
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex SkeletonBonesModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.size())
            return {};
         if (col < 0 || col >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex SkeletonBonesModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex SkeletonBonesModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int SkeletonBonesModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.size();
      }
      /*virtual*/ int SkeletonBonesModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant SkeletonBonesModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_data.size())
            return {};
         auto& src = this->_data[index.row()];
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
            case Qt::UserRole:
            case Qt::EditRole:
               return src.display_name;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags SkeletonBonesModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant SkeletonBonesModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      return tr("Bone Name");
   }
#pragma endregion

std::unique_ptr<nifDK::file> SkeletonBonesModel::_load_nif(std::filesystem::path&& path) {
   std::unique_ptr<dovah::bsa_archived_file> file(dovahkit::subsystems::assets::get().lookup_game_asset(path));
   if (!file)
      return nullptr;
   auto nif = std::make_unique<nifDK::file>();
   nif->read((void*)file->data(), file->size());
   return nif;
}
void SkeletonBonesModel::_make_base_node_item(const std::string& name) {
   if (!name.empty()) {
      this->_base_node_name = name;
      auto& item = this->_data.emplace_back();
      item.name         = name;
      item.display_name = QString::fromStdString(item.name);
   }
}

int SkeletonBonesModel::baseNodeRow() const {
   if (this->_base_node_name.empty())
      return -1;
   for (size_t i = 0; i < this->_data.size(); ++i) {
      if (this->_data[i].name == this->_base_node_name)
         return i;
   }
   return -1;
}
bool SkeletonBonesModel::hasBone(std::string_view name) const noexcept {
   for (const auto& bone : this->_data)
      if (bone.name == name)
         return true;
   return false;
}

void SkeletonBonesModel::resetFromSkeleton(std::string_view relative_to_meshes) {
   this->beginResetModel();
   this->_data.clear();
   this->_base_node_name.clear();
   if (relative_to_meshes.empty()) {
      this->endResetModel();
      return;
   }

   std::filesystem::path path;
   std::string base_node_name;
   {
      std::string path_str;
      path_str.reserve(std::string_view("Meshes").size() + 1 + relative_to_meshes.size());
      path_str += std::string_view("Meshes");
      if (!relative_to_meshes.empty()) {
         if (relative_to_meshes[0] != '/' && relative_to_meshes[0] != '\\')
            path_str += '\\';
         path_str += relative_to_meshes;
      }

      base_node_name = "BASE " + path_str;
      path           = std::move(path_str);
   }
   this->_make_base_node_item(base_node_name);

   auto nif_ptr = this->_load_nif(std::move(path));
   if (!nif_ptr) {
      ;
   } else if (nif_ptr->read_error().code != nifDK::default_notice_code) {
      #if _DEBUG
         __debugbreak();
      #endif
   } else {
      nif_ptr->root_node->for_self_and_subtree([this](nifDK::block_types::NiNode* node) {
         if (!node->name.empty()) {
            auto& item = this->_data.emplace_back();
            item.name         = node->name;
            item.display_name = QString::fromStdString(item.name);
         }
      });
      std::sort(
         this->_data.begin(),
         this->_data.end(),
         [](const auto& a, const auto& b) {
            return a.display_name.compare(b.display_name) < 0;
         }
      );
   }
   this->endResetModel();
}