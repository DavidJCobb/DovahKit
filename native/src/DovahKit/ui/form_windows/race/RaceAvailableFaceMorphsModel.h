#pragma once
#include <array>
#include <vector>
#include <QAbstractItemModel>
#include "dovah/data/sex.h"

namespace dovah {
   namespace loaded_forms {
      class Race;
   }
}

class RaceAvailableFaceMorphsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      RaceAvailableFaceMorphsModel(QObject* parent = nullptr);

      static constexpr const size_t ColumnCount = 1;

      enum class IndexedMorphType {
         Nose,
         Brow,
         Eyes,
         Lips,
      };
      
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

   protected:
      void _update_morph_counts();

   public:
      void initializeFrom(const dovah::loaded_forms::Race&, dovah::sex);
      void commitTo(dovah::loaded_forms::Race&, dovah::sex) const;

   protected:
      union _ {
         ~_() { all.~array(); }

         std::array<std::vector<bool>, 4> all = {};
         struct {
            std::vector<bool> nose;
            std::vector<bool> brow;
            std::vector<bool> eyes;
            std::vector<bool> lips;
         };
      } _data;
};