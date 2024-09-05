#include "./RaceAvailableFaceMorphsModel.h"
#include <variant>
#include "helpers/string/strieq_ascii.h"
#include "dovah/forms/Race.h"
#include "editor/subsystems/game_settings/core.h"

namespace {
   constexpr const size_t default_morph_count_per_type = 10;

   constexpr const auto desired_game_settings = std::array{
      "iBrowMorphCount",
      "iEyesMorphCount",
      "iLipMorphCount",
      "iNoseMorphCount",
   };
}

RaceAvailableFaceMorphsModel::RaceAvailableFaceMorphsModel(QObject* parent) : QAbstractItemModel(parent) {
   this->_update_morph_counts();

   auto& gss = dovahkit::subsystems::game_settings::core::get();
   QObject::connect(&gss, &std::decay_t<decltype(gss)>::settingValueChanged, this, [this](const char* name) {
      for (auto* desired : desired_game_settings) {
         if (cobb::strieq_ascii(name, desired)) {
            this->_update_morph_counts();
            return;
         }
      }
   });
}
      
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex RaceAvailableFaceMorphsModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || column < 0 || parent.isValid())
            return {};
         if (column >= ColumnCount)
            return {};
         if (row >= this->rowCount(parent))
            return {};

         return this->createIndex(row, column, nullptr);
      }
      /*virtual*/ QModelIndex RaceAvailableFaceMorphsModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex RaceAvailableFaceMorphsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int RaceAvailableFaceMorphsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         size_t size = 0;
         for (auto& list : this->_data.all)
            size += list.size();
         return size;
      }
      /*virtual*/ int RaceAvailableFaceMorphsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant RaceAvailableFaceMorphsModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};

         size_t group;
         bool   valid   = false;
         bool   checked = false;

         size_t i = index.row();
         for (group = 0; group < this->_data.all.size(); ++group) {
            auto& list = this->_data.all[group];
            if (i < list.size()) {
               valid   = true;
               checked = list[i];
               break;
            }
            i -= list.size();
         }
         if (!valid)
            return {};

         switch (role) {
            case Qt::CheckStateRole:
               return checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               {
                  QString name;
                  switch (group) {
                     case 0:
                        name = "NoseType%1";
                        break;
                     case 1:
                        name = "BrowType%1";
                        break;
                     case 2:
                        name = "EyesType%1";
                        break;
                     case 3:
                        name = "LipType%1";
                        break;
                  }
                  name = name.arg(i);
                  return name;
               }
               break;
         }

         return {};
      }
      /*virtual*/ Qt::ItemFlags RaceAvailableFaceMorphsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         flags |= Qt::ItemFlag::ItemIsUserCheckable;
         return flags;
      }
      #pragma region Write-access
         /*virtual*/ bool RaceAvailableFaceMorphsModel::setData(const QModelIndex& index, const QVariant& value, int role) /*override*/ {
            if (role != Qt::CheckStateRole)
               return false;

            if (!index.isValid())
               return false;

            size_t i = index.row();
            for (size_t group = 0; group < this->_data.all.size(); ++group) {
               auto& list = this->_data.all[group];
               if (i < list.size()) {
                  list[i] = value.toBool();
                  emit dataChanged(index, index, { role });
                  return true;
               }
               i -= list.size();
            }
            return false;
         }
      #pragma endregion
   #pragma endregion
   /*virtual*/ QVariant RaceAvailableFaceMorphsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      if (section == 0)
         return tr("Morph");
      return {};
   }
#pragma endregion

void RaceAvailableFaceMorphsModel::_update_morph_counts() {
   auto& gss = dovahkit::subsystems::game_settings::core::get();

   auto _handle = [&gss](
      const char* gmst,
      std::vector<bool>& dst
   ) {
      size_t morph_count = default_morph_count_per_type;
      {
         auto variant = gss.get_setting_value("iLipMorphCount");
         if (std::holds_alternative<int32_t>(variant))
            morph_count = std::get<int32_t>(variant);
      }
      dst.resize(morph_count);
   };

   this->beginResetModel();
   _handle("iBrowMorphCount", this->_data.brow);
   _handle("iEyesMorphCount", this->_data.eyes);
   _handle("iLipMorphCount",  this->_data.lips);
   _handle("iNoseMorphCount", this->_data.nose);
   this->endResetModel();
}

void RaceAvailableFaceMorphsModel::initializeFrom(const dovah::loaded_forms::Race& race, dovah::sex sex) {
   auto& src = race.by_sex[sex].head_data.morphs;

   auto _clear = [this](const auto& src_bitset, auto& dst_bitset) {
      size_t end = std::min(src_bitset.size(), dst_bitset.size());
      for (size_t i = 0; i < end; ++i)
         dst_bitset[i] = src_bitset[i];
   };
   _clear(src.brows,  this->_data.brow);
   _clear(src.eyes,   this->_data.eyes);
   _clear(src.noses,  this->_data.nose);
   _clear(src.mouths, this->_data.lips);
}
void RaceAvailableFaceMorphsModel::commitTo(dovah::loaded_forms::Race& race, dovah::sex sex) const {
   auto& dst = race.by_sex[sex].head_data.morphs;

   auto _clear = [this](auto& dst_bitset, const auto& src_bitset) {
      dst_bitset.clear();

      size_t end = std::min(src_bitset.size(), dst_bitset.size());
      for (size_t i = 0; i < end; ++i)
         if (src_bitset[i])
            dst_bitset[i] = true;
   };
   _clear(dst.brows,  this->_data.brow);
   _clear(dst.eyes,   this->_data.eyes);
   _clear(dst.noses,  this->_data.nose);
   _clear(dst.mouths, this->_data.lips);
}