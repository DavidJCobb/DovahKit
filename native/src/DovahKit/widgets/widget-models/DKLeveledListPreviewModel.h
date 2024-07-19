#pragma once
#include <vector>
#include <QAbstractItemModel>
#include "dovah/utils/leveled_list_preview.h"

namespace dovah {
   class form_stub;
}

class DKLeveledListPreviewModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      static constexpr const float normal_item_health  = 1.0F;
      static constexpr const float health_display_mult = 100; // multiply health by this amount when displaying it in the UI

      struct GeneratedObject {
         dovah::form_stub* form = nullptr;
         size_t level  = 0;
         size_t count  = 0;
         float  health = normal_item_health;
         dovah::form_stub* owner = nullptr;

         // Cached data, to avoid doing UI-side string conversions every repaint:
         struct {
            QString editorID;
            QString ownerEditorID;
         } cached;
      };

      struct Column {
         Column() = delete;
         enum {
            Count = 0,
            Form  = 1,

            _COUNT_ALWAYS,

            Owner = _COUNT_ALWAYS,
            Health,

            _COUNT_IF_ITEMS,
         };
      };

   public:
      DKLeveledListPreviewModel(QObject* parent = nullptr);
      ~DKLeveledListPreviewModel();
      
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override final;
            virtual int         columnCount(const QModelIndex& item = {}) const override final;

            virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
         #pragma endregion
      #pragma endregion

      void overwrite(const std::vector<dovah::leveled_list_preview::entry>& src);

      constexpr bool showsContainerItemFields() const noexcept {
         return this->_show_container_item_fields;
      }
      void setShowsContainerItemFields(bool);

   protected:
      bool _show_container_item_fields = true;
      std::vector<GeneratedObject> _items;

      void _clear();
};