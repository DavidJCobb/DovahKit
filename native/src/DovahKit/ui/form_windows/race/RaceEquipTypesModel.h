#pragma once
#include <array>
#include <QAbstractItemModel>

namespace dovah::loaded_forms {
   class Race;
}

class RaceEquipTypesModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      RaceEquipTypesModel(QObject* parent = nullptr);

      static constexpr const size_t ColumnCount = 1;
      
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override;
            virtual int         columnCount(const QModelIndex& parent = {}) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
            #pragma region Write-access
               virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
            #pragma endregion
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      void initializeFrom(const dovah::loaded_forms::Race&);
      void commitTo(dovah::loaded_forms::Race&) const;

   protected:
      struct Data {
         bool enabled = false;
      };
      std::array<Data, 13> _data = {};
};