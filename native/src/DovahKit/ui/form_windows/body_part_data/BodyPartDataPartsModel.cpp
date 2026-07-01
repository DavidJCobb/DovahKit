#include "./BodyPartDataPartsModel.h"
#include "editor/core.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "./SkeletonBonesModel.h"

BodyPartDataPartsModel::BodyPartDataPartsModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
      for (size_t i = 0; i < this->_data.parts.size(); ++i) {
         auto& part    = this->_data.parts[i];
         bool  changed = false;
         
         #define CASE(n) if (part.n == stub) { changed = true; part.n = nullptr; }
         CASE(gore.explodable.debris);
         CASE(gore.explodable.explosion);
         CASE(gore.explodable.impact_data_set);
         CASE(gore.severable.debris);
         CASE(gore.severable.explosion);
         CASE(gore.severable.impact_data_set);
         #undef CASE

         if (changed) {
            emit dataChanged(this->index(i, 0), this->index(i, ColumnCount - 1));
         }
      }
   });
}

void BodyPartDataPartsModel::_reset_invalid_bone_names(bool silent) {
   if (!this->_data.bones_model)
      return;
   auto& parts     = this->_data.parts;
   auto& base_node = this->_data.bones_model->baseNodeName();
   for (size_t i = 0; i < parts.size(); ++i) {
      auto& part    = parts[i];
      bool  changed = false;

      #define CASE(n) if (!this->_data.bones_model->hasBone(part.nodes.n)) { changed = true; part.nodes.n = base_node; }
      CASE(gore_effect);
      CASE(ik_start);
      CASE(main);
      CASE(vats_target);
      #undef CASE

      if (changed && !silent) {
         emit dataChanged(this->index(i, 0), this->index(i, ColumnCount - 1));
      }
   }
}

void BodyPartDataPartsModel::importData(loaded_form_type& src) {
   this->beginResetModel();
   this->_data.parts.clear();
   {
      const auto& src_list = src.parts;
      auto&       dst_list = this->_data.parts;
      const auto  size     = src_list.size();
      dst_list.clear();
      dst_list.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& src_part = src_list[i];
         auto& dst_part = dst_list[i];
         
         #define ASSIGN_FORM(_field)  dst_part._field = src_part._field.get_form_stub();
         #define ASSIGN_VALUE(_field) dst_part._field = src_part._field;

         ASSIGN_VALUE(name);
         ASSIGN_VALUE(flags);
         ASSIGN_VALUE(limb);
         ASSIGN_VALUE(combat);
         ASSIGN_VALUE(headtracking_max_angle);
         ASSIGN_VALUE(gore.effect_positioning);
         {
            ASSIGN_FORM(gore.explodable.debris);
            ASSIGN_FORM(gore.explodable.explosion);
            ASSIGN_FORM(gore.explodable.impact_data_set);
            ASSIGN_VALUE(gore.explodable.chance);
            ASSIGN_VALUE(gore.explodable.debris_count);
            ASSIGN_VALUE(gore.explodable.debris_scale);
            ASSIGN_VALUE(gore.explodable.decal_count);
            dst_part.gore.explodable.limb_replacement.model.clone_from(src_part.gore.explodable.limb_replacement.model);
            ASSIGN_VALUE(gore.explodable.limb_replacement.scale);
         }
         {
            ASSIGN_FORM(gore.severable.debris);
            ASSIGN_FORM(gore.severable.explosion);
            ASSIGN_FORM(gore.severable.impact_data_set);
            ASSIGN_VALUE(gore.severable.debris_count);
            ASSIGN_VALUE(gore.severable.debris_scale);
            ASSIGN_VALUE(gore.severable.decal_count);
         }
         ASSIGN_VALUE(nodes);
         ASSIGN_VALUE(pose_matching);

         #undef ASSIGN_FORM
         #undef ASSIGN_VALUE
      }
   }
   if (this->_data.bones_model) {
      this->_data.bones_model->resetFromSkeleton(src.model.model_path);
   }
   this->endResetModel();
}
void BodyPartDataPartsModel::exportData(loaded_form_type& dst) const {
   const auto& src_list = this->_data.parts;
   auto&       dst_list = dst.parts;
   const auto  size     = src_list.size();
   for (auto& dst_part : dst_list)
      dst_part.clear(dst);
   dst_list.clear();
   dst_list.resize(size);
   for (size_t i = 0; i < size; ++i) {
      auto& src_part = src_list[i];
      auto& dst_part = dst_list[i];
         
      #define ASSIGN_FORM(_field)  dst_part._field.set(dst, src_part._field);
      #define ASSIGN_VALUE(_field) dst_part._field = src_part._field;

      ASSIGN_VALUE(name);
      ASSIGN_VALUE(flags);
      ASSIGN_VALUE(limb);
      ASSIGN_VALUE(combat);
      ASSIGN_VALUE(headtracking_max_angle);
      ASSIGN_VALUE(gore.effect_positioning);
      {
         ASSIGN_FORM(gore.explodable.debris);
         ASSIGN_FORM(gore.explodable.explosion);
         ASSIGN_FORM(gore.explodable.impact_data_set);
         ASSIGN_VALUE(gore.explodable.chance);
         ASSIGN_VALUE(gore.explodable.debris_count);
         ASSIGN_VALUE(gore.explodable.debris_scale);
         ASSIGN_VALUE(gore.explodable.decal_count);
         dst_part.gore.explodable.limb_replacement.model.clone_from(src_part.gore.explodable.limb_replacement.model);
         ASSIGN_VALUE(gore.explodable.limb_replacement.scale);
      }
      {
         ASSIGN_FORM(gore.severable.debris);
         ASSIGN_FORM(gore.severable.explosion);
         ASSIGN_FORM(gore.severable.impact_data_set);
         ASSIGN_VALUE(gore.severable.debris_count);
         ASSIGN_VALUE(gore.severable.debris_scale);
         ASSIGN_VALUE(gore.severable.decal_count);
      }
      ASSIGN_VALUE(nodes);
      ASSIGN_VALUE(pose_matching);

      #undef ASSIGN_FORM
      #undef ASSIGN_VALUE
   }
}

SkeletonBonesModel* BodyPartDataPartsModel::bonesModel() const {
   return this->_data.bones_model;
}
void BodyPartDataPartsModel::setBonesModel(SkeletonBonesModel* v) {
   if (this->_data.bones_model == v)
      return;
   this->_data.bones_model = v;
}

void BodyPartDataPartsModel::resetInvalidBoneNames() {
   _reset_invalid_bone_names(false);
}

const BodyPartDataPartsModel::loaded_item_type* BodyPartDataPartsModel::bodyPart(size_t row) const {
   if (row >= this->_data.parts.size())
      return nullptr;
   return &this->_data.parts[row];
}
void BodyPartDataPartsModel::replaceBodyPart(size_t row, const loaded_item_type& data) {
   if (row >= this->_data.parts.size())
      return;
   this->_data.parts[row] = data;
   emit dataChanged(this->index(row, 0), this->index(row, ColumnCount - 1));
}
      
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex BodyPartDataPartsModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || column < 0 || column >= ColumnCount)
            return {};
         if (parent.isValid())
            return {};
         if (row >= this->rowCount())
            return {};
         return this->createIndex(row, column, nullptr);
      }
      /*virtual*/ QModelIndex BodyPartDataPartsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex BodyPartDataPartsModel::sibling(int row, int column, const QModelIndex& qmi) const /*override*/ {
         return this->index(row, column);
      }
      /*virtual*/ int BodyPartDataPartsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         if (parent.isValid())
            return 0;
         return this->_data.parts.size();
      }
      /*virtual*/ int BodyPartDataPartsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant BodyPartDataPartsModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};

         auto row = index.row();
         if (row < 0 || row >= this->_data.parts.size())
            return {};

         auto& part = this->_data.parts[row];
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (index.column()) {
                  case Column::Name:
                     return dovahkit::subsystems::game_localized_strings::core::get().convert_localized_string(part.name);
                  case Column::Limb:
                     switch (part.limb) {
                        case dovah::limb::eye:
                           return tr("Eye");
                        case dovah::limb::fly_grab:
                           return tr("FlyGrab");
                        case dovah::limb::head:
                           return tr("Head");
                        case dovah::limb::look_at:
                           return tr("Look At");
                        case dovah::limb::saddle:
                           return tr("Saddle");
                        case dovah::limb::torso:
                           return tr("Torso");
                     }
                     break;
                  case Column::MainNode:
                     return QString::fromStdString(part.nodes.main);
                  case Column::TargetNode:
                     return QString::fromStdString(part.nodes.vats_target);
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags BodyPartDataPartsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
      }
   #pragma endregion
   /*virtual*/ QVariant BodyPartDataPartsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      switch (role) {
         case Qt::DisplayRole:
         case Qt::ToolTipRole:
            switch (section) {
               case Column::Name:
                  return tr("Name");
               case Column::Limb:
                  return tr("Limb");
               case Column::MainNode:
                  return tr("Main Node");
               case Column::TargetNode:
                  return tr("VATS Node");
            }
            break;
      }
      return {};
   }
   #pragma region Editing
      /*virtual*/ bool BodyPartDataPartsModel::insertRows(int row, int count, const QModelIndex& parent) /*override*/ {
         if (parent.isValid())
            return false;
         auto& list = this->_data.parts;
         if (row < 0 || row > list.size() || count <= 0)
            return false;
         this->beginInsertRows({}, row, row + count - 1);
         for (size_t i = 0; i < count; ++i)
            list.emplace(list.begin() + row);
         this->endInsertRows();
         return true;
      }
      /*virtual*/ bool BodyPartDataPartsModel::removeRows(int row, int count, const QModelIndex& parent) /*override*/ {
         if (parent.isValid())
            return false;
         auto& list = this->_data.parts;
         if (row < 0 || row >= list.size() || count <= 0)
            return false;
         this->beginRemoveRows(parent, row, row + count - 1);
         list.erase(list.begin() + row, list.begin() + row + count);
         this->endRemoveRows();
         return true;
      }
   #pragma endregion
#pragma endregion