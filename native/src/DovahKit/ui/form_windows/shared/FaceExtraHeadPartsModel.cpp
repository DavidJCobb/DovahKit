#include "./FaceExtraHeadPartsModel.h"
#include "dovah/form_stub.h"
#include "dovah/utils/form_list_contains.h"
#include "editor/subsystems/form_info_cache/core.h"
#include "editor/core.h"

namespace {
   namespace form_info_cache {
      using namespace dovahkit::subsystems::form_info_cache;
   }
}

void FaceExtraHeadPartsModel::Item::recache_type_name() {
   auto& dst = this->cached.type;
   switch (this->type) {
      case HeadPartType::eyebrows:
         dst = tr("Brows");
         break;
      case HeadPartType::eyes:
         dst = tr("Eyes");
         break;
      case HeadPartType::face:
         dst = tr("Face");
         break;
      case HeadPartType::facial_hair:
         dst = tr("Facial Hair");
         break;
      case HeadPartType::hair:
         dst = tr("Hair");
         break;
      case HeadPartType::misc:
         dst = tr("Misc");
         break;
      case HeadPartType::scar:
         dst = tr("Scar");
         break;
      default:
         dst = "";
         break;
   }
}

//

FaceExtraHeadPartsModel::FaceExtraHeadPartsModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& fic    = form_info_cache::core::get();
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->beginResetModel();
      for (auto& item : this->_items) {
         item.stub   = nullptr;
         item.cached = {};
      }
      this->_items.clear();
      this->endResetModel();
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      auto&  list = this->_items;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.stub == stub) {
            item.cached.editorID = QString::fromStdString(stub->editorID);

            auto qmi = this->index(i, 1);
            emit dataChanged(qmi, qmi);
         }
      }
   });
   QObject::connect(&fic, &std::decay_t<decltype(fic)>::cachedHeadPartChanged, this, [this, &fic](dovah::form_stub& stub) {
      const form_info_cache::cached_head_part_info* info = nullptr;

      auto&  list = this->_items;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.stub != &stub)
            continue;
         
         if (!info) {
            info = fic.get_head_part_info(stub);
            if (!info)
               continue;
         }
         //
         // TODO: Have the model track a race requirement and if one is set, and the 
         //       HeadPart was changed not to allow that race, then remove it.
         // 
         // TODO: Ditto for sex, actually
         //
         if (item.type == info->type)
            continue;

         item.type = info->type;
         item.recache_type_name();

         auto qmi = this->index(i, 0);
         emit dataChanged(qmi, qmi);
      }
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      auto&  list = this->_items;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& item = list[i];
         if (item.stub == stub) {
            this->beginRemoveRows({}, i, i);
            list.erase(list.begin() + i);
            --i;
            --size;
            this->endRemoveRows();
         }
      }
   });
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex FaceExtraHeadPartsModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_items.size())
            return {};
         if (column < 0 || column >= 2)
            return {};
         return this->createIndex(row, column, nullptr);
      }
      /*virtual*/ QModelIndex FaceExtraHeadPartsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex FaceExtraHeadPartsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int FaceExtraHeadPartsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         if (parent.isValid())
            return {};
         return this->_items.size();
      }
      /*virtual*/ int FaceExtraHeadPartsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return 2;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant FaceExtraHeadPartsModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         size_t i = index.row();
         if (i >= this->_items.size())
            return {};
         auto& item = this->_items[i];
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (index.column()) {
                  case 0:
                     return item.cached.type;
                  case 1:
                     return item.cached.editorID;
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags FaceExtraHeadPartsModel::flags(const QModelIndex& index) const /*override*/ {
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant FaceExtraHeadPartsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      switch (section) {
         case 0:
            return tr("Type");
         case 1:
            return tr("Editor ID");
      }
      return {};
   }
#pragma endregion

void FaceExtraHeadPartsModel::appendHeadPart(dovah::form_stub& stub) {
   if (this->containsHeadPart(stub))
      return;
   auto&  list = this->_items;
   size_t size = list.size();
   this->beginInsertRows({}, size, size);
   //
   auto&  item = list.emplace_back();
   item.stub = &stub;
   {
      auto& fic  = form_info_cache::core::get();
      auto* info = fic.get_head_part_info(stub);
      if (info)
         item.type = info->type;
   }
   item.cached.editorID = QString::fromStdString(stub.editorID);
   item.recache_type_name();
   //
   this->endInsertRows();
}
bool FaceExtraHeadPartsModel::containsHeadPart(const dovah::form_stub& stub) const {
   for (auto& item : this->_items)
      if (item.stub == &stub)
         return true;
   return false;
}
void FaceExtraHeadPartsModel::removeHeadPart(const dovah::form_stub& stub) {
   auto&  list = this->_items;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto& item = list[i];
      if (item.stub != &stub)
         continue;
      this->beginRemoveRows({}, i, i);
      list.erase(list.begin() + i);
      --i;
      --size;
      this->endRemoveRows();
   }
}
void FaceExtraHeadPartsModel::replaceAllHeadParts(const std::vector<dovah::form_stub*>& src) {
   this->beginResetModel();
   this->_items.clear();
   this->_items.reserve(src.size());
   for (size_t i = 0; i < src.size(); ++i) {
      auto* stub = src[i];
      if (!stub)
         continue;

      bool already_contains = false;
      for (size_t j = 0; j < i; ++j) {
         if (this->_items[j].stub == stub) {
            already_contains = true;
            break;
         }
      }
      if (already_contains)
         continue;

      auto& item = this->_items.emplace_back();
      item.stub = stub;
      {
         auto& fic = form_info_cache::core::get();
         auto* info = fic.get_head_part_info(*stub);
         if (info)
            item.type = info->type;
      }
      item.cached.editorID = QString::fromStdString(stub->editorID);
      item.recache_type_name();
   }
   this->endResetModel();
}
//
dovah::form_stub* FaceExtraHeadPartsModel::headPart(size_t i) const {
   if (i >= this->_items.size())
      return nullptr;
   return this->_items[i].stub;
}
std::vector<dovah::form_stub*> FaceExtraHeadPartsModel::headParts() const {
   std::vector<dovah::form_stub*> out;
   out.reserve(this->_items.size());
   for (auto& src : this->_items)
      out.push_back(src.stub);
   return out;
}

void FaceExtraHeadPartsModel::filterForRace(dovah::form_stub* race) {
   if (!race)
      return;
   auto&  fic  = dovahkit::subsystems::form_info_cache::core::get();
   auto&  list = this->_items;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto& item = list[i];
      auto* stub = item.stub;
      if (!stub)
         continue;

      auto* info = fic.get_head_part_info(*stub);
      if (!info)
         continue;

      if (!info->race_list)
         continue;

      if (!dovah::form_list_contains(*info->race_list, *race)) {
         this->beginRemoveRows({}, i, i);
         list.erase(list.begin() + i);
         --i;
         --size;
         this->endRemoveRows();
      }
   }
}
void FaceExtraHeadPartsModel::filterForSex(dovah::sex sex) {
   auto&  fic  = dovahkit::subsystems::form_info_cache::core::get();
   auto&  list = this->_items;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto& item = list[i];
      auto* stub = item.stub;
      if (!stub)
         continue;
      auto* info = fic.get_head_part_info(*stub);
      if (!info)
         continue;

      if (info->sex.has_value() && info->sex.value() != sex) {
         this->beginRemoveRows({}, i, i);
         list.erase(list.begin() + i);
         --i;
         --size;
         this->endRemoveRows();
      }
   }
}