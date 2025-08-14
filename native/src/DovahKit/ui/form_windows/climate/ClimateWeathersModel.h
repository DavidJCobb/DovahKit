#pragma once
#include <cstdint>
#include "ui/models/DKGenericListModel.h"

namespace dovah {
   class form_stub;
}

struct ClimateWeathersModelNode {
   dovah::form_stub* weather = nullptr;
   dovah::form_stub* global  = nullptr;
   int32_t chance = 0;

   struct {
      QString global_editor_id;
      QString weather_editor_id;
   } cached;
};

class ClimateWeathersModel : public DKGenericListModel<ClimateWeathersModel, ClimateWeathersModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            Weather,
            Chance,
            Global,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override

   public:
      ClimateWeathersModel(QObject* parent);

      using DKGenericListModel::clear;
      using DKGenericListModel::deleteItems;

      #pragma region Overrides
         QVariant data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const;
         Qt::ItemFlags flags_of(const node_type&, size_t column) const;
         constexpr bool allow_inbound_drag_and_drop() const {
            return true;
         }

         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
         
         #pragma region Drag-and-drop
            virtual bool canDropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) const override;
            virtual bool dropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) override;
            virtual QStringList mimeTypes() const override;
            virtual Qt::DropActions supportedDropActions() const override;
         #pragma endregion
      #pragma endregion

      QModelIndex create();
      QModelIndex overwrite(int row, const node_type& src);
      const node_type* item(int row) const;

      void overwriteAllItems(const std::vector<node_type>& src);

      inline const bool containsWeather(const dovah::form_stub* stub) const noexcept {
         for (auto* node : this->_nodes)
            if (node->weather == stub)
               return true;
         return false;
      }
};
