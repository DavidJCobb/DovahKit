#pragma once
#include <array>
#include <QAbstractItemModel>

namespace dovah {
   namespace loaded_forms {
      namespace components {
         struct biped_object;
      }
      class Race;
   }
   class form_stub;
}

class BipedObjectSlotsToggleModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      static constexpr const size_t slot_count = 32;

   public:
      BipedObjectSlotsToggleModel(QObject* parent = nullptr);

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
               virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
            #pragma endregion
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      void setSlotNamesFrom(dovah::form_stub&);

      void importFlags(const dovah::loaded_forms::components::biped_object&);
      void exportFlags(dovah::loaded_forms::components::biped_object&) const;

   protected:
      struct Slot {
         QString name;
         bool    enabled = false;
      };

      dovah::form_stub* _names_set_from = nullptr;
      std::array<Slot, slot_count> _slots = {};
};