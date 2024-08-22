#pragma once
#include <array>
#include <optional>
#include <QAbstractItemModel>
#include "dovah/data/sex.h"
#include "dovah/forms/HeadPart.h"

namespace dovah {
   class form_stub;
}

//
// A model for working with an ActorBase or Race's "Base Head Parts" list in the UI.
//
class FaceBaseHeadPartsModel final : public QAbstractItemModel {
   Q_OBJECT;
   public:
      enum class Slot {
         Brows,
         Eyes,
         Face,
         FacialHair,
         Hair,
      };
      static constexpr const size_t slot_count = 5;

   public:
      FaceBaseHeadPartsModel(QObject* parent = nullptr);

      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
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

      constexpr std::optional<Slot> slotAt(size_t row) const noexcept {
         if (row < slot_count)
            return (Slot)row;
         return {};
      }

      static constexpr dovah::head_part_type slotToType(Slot s) noexcept {
         using hpt = dovah::head_part_type;
         switch (s) {
            case Slot::Brows: return hpt::eyebrows;
            case Slot::Eyes: return hpt::eyes;
            case Slot::Face: return hpt::face;
            case Slot::FacialHair: return hpt::facial_hair;
            case Slot::Hair: return hpt::hair;
         }
         return hpt::misc;
      }

      dovah::form_stub* headPartFor(Slot) const;
      void setHeadPartFor(Slot, dovah::form_stub*);

      void filterForRace(dovah::form_stub* race);
      void filterForSex(dovah::sex);

   protected:
      struct SlotValue {
         dovah::form_stub* stub = nullptr;
         struct {
            QString editorID;
         } cached;
      };

      union _ {
         ~_() {
            this->list.~array();
         }

         struct {
            SlotValue brows;
            SlotValue eyes;
            SlotValue face;
            SlotValue facial_hair;
            SlotValue hair;
         };
         std::array<SlotValue, 5> list = {};
      } _data;
};