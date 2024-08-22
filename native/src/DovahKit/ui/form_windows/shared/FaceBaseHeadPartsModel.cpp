#include "./FaceBaseHeadPartsModel.h"
#include "dovah/form_stub.h"
#include "dovah/utils/form_list_contains.h"
#include "editor/core.h"
#include "editor/helpers/form_stub_drag_drop.h"
#include "editor/subsystems/form_info_cache/core.h"

namespace {
   namespace form_info_cache {
      using namespace dovahkit::subsystems::form_info_cache;
   }

   std::optional<FaceBaseHeadPartsModel::Slot> _from_backend_enum(dovah::head_part_type t) {
      switch (t) {
         using enum FaceBaseHeadPartsModel::Slot;
         case dovah::head_part_type::eyebrows:
            return Brows;
         case dovah::head_part_type::eyes:
            return Eyes;
         case dovah::head_part_type::face:
            return Face;
         case dovah::head_part_type::facial_hair:
            return FacialHair;
         case dovah::head_part_type::hair:
            return Hair;
      }
      return {};
   }
}

FaceBaseHeadPartsModel::FaceBaseHeadPartsModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      for (auto& item : this->_data.list) {
         item.stub   = nullptr;
         item.cached = {};
      }
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      auto& list = this->_data.list;
      for (size_t i = 0; i < list.size(); ++i) {
         auto& item = list[i];
         if (item.stub == stub) {
            item.cached.editorID = QString::fromStdString(stub->editorID);

            auto qmi = this->index(i, 1);
            emit dataChanged(qmi, qmi);
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      auto& list = this->_data.list;
      for (size_t i = 0; i < list.size(); ++i) {
         auto& item = list[i];
         if (item.stub == stub) {
            item.stub = nullptr;
            item.cached.editorID = "";

            auto qmi = this->index(i, 1);
            emit dataChanged(qmi, qmi);
         }
      }
   });
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex FaceBaseHeadPartsModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (column < 0 || column >= 2)
            return {};
         if (row < 0 || row >= slot_count)
            return {};
         return this->createIndex(row, column, nullptr);
      }
      /*virtual*/ QModelIndex FaceBaseHeadPartsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex FaceBaseHeadPartsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int FaceBaseHeadPartsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         if (parent.isValid())
            return {};
         return slot_count;
      }
      /*virtual*/ int FaceBaseHeadPartsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return 2;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant FaceBaseHeadPartsModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         size_t i = index.row();
         if (i >= this->_data.list.size())
            return {};
         auto& item = this->_data.list[i];
         switch (index.column()) {
            case 0:
               switch ((Slot)i) {
                  case Slot::Brows:
                     return tr("Brows");
                  case Slot::Eyes:
                     return tr("Eyes");
                  case Slot::Face:
                     return tr("Face");
                  case Slot::FacialHair:
                     return tr("Facial Hair");
                  case Slot::Hair:
                     return tr("Hair");
               }
               break;
            case 1:
               return item.cached.editorID;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags FaceBaseHeadPartsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return Qt::ItemFlag::ItemIsDropEnabled;
         return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
      }
   #pragma endregion
   /*virtual*/ QVariant FaceBaseHeadPartsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
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
   #pragma region Drag-and-drop
      bool FaceBaseHeadPartsModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const {
         if (action != Qt::DropAction::CopyAction)
            return false;
         if (!data)
            return false;
         if (!data->hasFormat(editor_helpers::form_stub_array_mime_type))
            return false;
         return true;
      }
      bool FaceBaseHeadPartsModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
         if (!this->canDropMimeData(data, action, row, column, parent))
            return false;
         if (action == Qt::IgnoreAction)
            return true;
         
         auto stubs = editor_helpers::form_stubs_from_mime_data(*data);
         if (stubs.empty())
            return true;

         std::array<bool, slot_count> changes = {};
         {
            auto& fic = form_info_cache::core::get();
            for (dovah::form_stub* stub : stubs) {
               if (!stub || stub->form_type != dovah::form_type::head_part)
                  continue;
               auto* info = fic.get_head_part_info(*stub);
               if (!info)
                  continue;

               auto slot = _from_backend_enum(info->type);
               if (!slot.has_value())
                  continue;

               if (auto* race = this->_last_filters.race) {
                  if (info->race_list) {
                     if (!dovah::form_list_contains(*info->race_list, *race))
                        continue;
                  }
               }
               if (this->_last_filters.sex.has_value()) {
                  if (info->sex.has_value() && info->sex.value() != this->_last_filters.sex.value())
                     continue;
               }

               auto i = (size_t)slot.value();
               changes[i] = true;
               this->_data.list[i].stub = stub;
            }
         }
         for (size_t i = 0; i < changes.size(); ++i) {
            if (!changes[i])
               continue;
            auto& item = this->_data.list[i];
            item.cached.editorID = QString::fromStdString(item.stub->editorID);

            auto qmi = this->index(i, 1, {});
            emit dataChanged(qmi, qmi);
         }
         return true;
      }
      QStringList FaceBaseHeadPartsModel::mimeTypes() const {
         return QStringList(QString(editor_helpers::form_stub_array_mime_type));
      }
      Qt::DropActions FaceBaseHeadPartsModel::supportedDropActions() const {
         return Qt::CopyAction;
      }
   #pragma endregion
#pragma endregion

dovah::form_stub* FaceBaseHeadPartsModel::headPartFor(Slot s) const {
   auto& list = this->_data.list;
   if ((size_t)s >= list.size())
      return nullptr;
   return list[(size_t)s].stub;
}
void FaceBaseHeadPartsModel::setHeadPartFor(Slot s, dovah::form_stub* stub) {
   auto& list = this->_data.list;
   if ((size_t)s >= list.size())
      return;
   auto& item = list[(size_t)s];
   if (item.stub == stub)
      return;
   item.stub = stub;
   item.cached.editorID = stub ? QString::fromStdString(stub->editorID) : "";

   auto qmi = this->index((size_t)s, 1);
   emit dataChanged(qmi, qmi);
}

void FaceBaseHeadPartsModel::filterForRace(dovah::form_stub* race) {
   if (!race)
      return;
   auto& fic  = dovahkit::subsystems::form_info_cache::core::get();
   auto& list = this->_data.list;
   for (size_t i = 0; i < list.size(); ++i) {
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
         item.stub   = nullptr;
         item.cached = {};

         auto qmi = this->index(i, 1);
         emit dataChanged(qmi, qmi);
      }
   }
}
void FaceBaseHeadPartsModel::filterForSex(dovah::sex sex) {
   auto& fic  = dovahkit::subsystems::form_info_cache::core::get();
   auto& list = this->_data.list;
   for (size_t i = 0; i < list.size(); ++i) {
      auto& item = list[i];
      auto* stub = item.stub;
      if (!stub)
         continue;
      auto* info = fic.get_head_part_info(*stub);
      if (!info)
         continue;

      if (info->sex.has_value() && info->sex.value() != sex) {
         item.stub   = nullptr;
         item.cached = {};

         auto qmi = this->index(i, 1);
         emit dataChanged(qmi, qmi);
      }
   }
}