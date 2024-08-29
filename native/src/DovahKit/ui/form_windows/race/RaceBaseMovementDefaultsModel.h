#pragma once
#include <array>
#include <QAbstractItemModel>

namespace dovah {
   namespace loaded_forms {
      class Race;
   }
   class form_stub;
}

class RaceBaseMovementDefaultsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      enum class MoveType {
         walk,
         run,
         swim,
         fly,
         sneak,
         sprint,

         __COUNT
      };
      static constexpr const size_t num_move_types = (size_t)MoveType::__COUNT;

   public:
      RaceBaseMovementDefaultsModel(QObject* parent = nullptr);

      struct Column {
         Column() = delete;
         enum type {
            Type,
            Form,

            __COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::__COUNT;
      
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
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      void initializeFrom(const dovah::loaded_forms::Race&);
      void commitTo(dovah::loaded_forms::Race&) const;

      dovah::form_stub* form(size_t row) const;
      void setForm(size_t row, dovah::form_stub*);

   protected:
      struct Slot {
         dovah::form_stub* stub = nullptr;
         QString           cached_editor_id;
      };
      std::array<Slot, num_move_types> _slots = {};
};