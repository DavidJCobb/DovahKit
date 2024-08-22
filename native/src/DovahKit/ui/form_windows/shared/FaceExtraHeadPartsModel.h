#pragma once
#include <vector>
#include <QAbstractItemModel>
#include "dovah/data/headparts.h"
#include "dovah/data/sex.h"

namespace dovah {
   class form_stub;
}

//
// A model for working with an ActorBase or Race's "Additional Head Parts" list in the UI.
//
class FaceExtraHeadPartsModel final : public QAbstractItemModel {
   Q_OBJECT;
   public:
      using HeadPartType = dovah::head_part_type;

   public:
      FaceExtraHeadPartsModel(QObject* parent = nullptr);

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
         #pragma region Drag-and-drop
            virtual bool canDropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) const override;
            virtual bool dropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) override;
            virtual QStringList mimeTypes() const override;
            virtual Qt::DropActions supportedDropActions() const override;
         #pragma endregion
      #pragma endregion

      void appendHeadPart(dovah::form_stub&);
      bool containsHeadPart(const dovah::form_stub&) const;
      void removeHeadPart(const dovah::form_stub&);
      void replaceAllHeadParts(const std::vector<dovah::form_stub*>&);
      //
      dovah::form_stub* headPart(size_t) const;
      [[nodiscard]] std::vector<dovah::form_stub*> headParts() const;

      void filterForRace(dovah::form_stub* race);
      void filterForSex(dovah::sex);

   protected:
      struct Item {
         dovah::form_stub* stub = nullptr;
         HeadPartType      type = HeadPartType::misc;
         struct {
            QString editorID;
            QString type;
         } cached;

         void recache_type_name();
      };
      std::vector<Item> _items;
      struct {
         dovah::form_stub* race = nullptr;
         std::optional<dovah::sex> sex;
      } _last_filters;

      std::vector<dovah::form_stub*> _extract_usable_head_parts(const QMimeData&) const;
};