#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include "dovah/form_types.h"

namespace dovah {
   namespace loaded_forms {
      namespace components {
         struct container_data;
      }
      class Form;
   }
   class form_stub;
}

class DKFormInventoryModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      static constexpr const float normal_item_health  = 1.0F;
      static constexpr const float health_display_mult = 100; // multiply health by this amount when displaying it in the UI

      struct InventoryObject {
         dovah::form_stub* form  = nullptr;
         uint16_t          count = 0;

         // COED fields:
         float health = 1;
         struct {
            dovah::form_stub* owner  = nullptr;
            dovah::form_stub* global = nullptr;
            int32_t           rank   = 0;
         } ownership;

         // Cached data, to avoid doing UI-side string conversions every repaint:
         struct {
            int32_t value = 0; // count of a single item
            QString editorID;
            QString ownerEditorID;
         } cached;
      };

      using loaded_form = dovah::loaded_forms::Form;
      using backend_type = dovah::loaded_forms::components::container_data;

      struct Column {
         Column() = delete;
         enum {
            Count,
            Form,
            Owner,
            Health,
            Value,

            _COUNT,
            _FIRST_EXTRA = Owner,
            _LAST_EXTRA  = Health,
            _COUNT_EXTRA = _LAST_EXTRA + 1 - _FIRST_EXTRA,
         };
      };

   public:
      DKFormInventoryModel(QObject* parent = nullptr);
      ~DKFormInventoryModel();
      
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
         #pragma region Editing
            virtual bool insertRows(int row, int count, const QModelIndex& parent = {}) override;
            virtual bool removeRows(int row, int count, const QModelIndex& parent = {}) override;
            #pragma region Drag-and-drop
               virtual bool canDropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) const override;
               virtual bool dropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) override;
               virtual QStringList mimeTypes() const override;
               virtual Qt::DropActions supportedDropActions() const override;
            #pragma endregion
         #pragma endregion
      #pragma endregion

      void importFrom(const backend_type& component);
      void commitTo(loaded_form& component_containing_form, backend_type& component) const;

      constexpr const InventoryObject* data(size_t row) const noexcept {
         if (row >= this->_items.size())
            return nullptr;
         return this->_items[row];
      }

      // Silently corrects any incorrect fields, e.g. forcing an owner of the wrong form 
      // type to None.
      void setData(size_t row, const InventoryObject&);

      bool allowsExtraData() const;
      void setAllowsExtraData(bool);

      bool allowsPseudoItems() const;
      void setAllowsPseudoItems(bool);

   protected:
      std::vector<InventoryObject*> _items;
      bool _allow_extra_data   = true;
      bool _allow_pseudo_items = true; // e.g. leveled lists

      bool _item_type_is_allowed(dovah::form_type) const;
      void _clear();
};