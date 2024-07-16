#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include "dovah/form_types.h"

namespace dovah {
   namespace loaded_forms {
      namespace components {
         class leveled_list;
      }
      class Form;
   }
   class form_stub;
}

class LeveledListModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      static constexpr const float normal_item_health = 100.0F;

      struct LeveledObject {
         dovah::form_stub* form = nullptr;
         uint16_t level  = 0;
         uint16_t count  = 0;

         // COED fields:
         float    health = normal_item_health;
         struct {
            dovah::form_stub* owner  = nullptr;
            dovah::form_stub* global = nullptr;
            int32_t           rank   = 0;
         } ownership;

         // Cached data, to avoid doing UI-side string conversions every repaint:
         struct {
            QString editorID;
            QString ownerEditorID;
         } cached;
      };

      using loaded_form  = dovah::loaded_forms::Form;
      using backend_type = dovah::loaded_forms::components::leveled_list;

      struct Column {
         Column() = delete;
         enum {
            Level = 0,
            Count = 1,
            Form  = 2,

            _COUNT_ALWAYS,

            Owner = _COUNT_ALWAYS,
            Health,

            _COUNT_IF_ITEMS,
         };
      };

   public:
      LeveledListModel(QObject* parent = nullptr);
      ~LeveledListModel();
      
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

            virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
         #pragma endregion
      #pragma endregion

      void importFrom(const backend_type& component);
      void commitTo(loaded_form& component_containing_form, backend_type& component) const;

      constexpr bool showsContainerItemFields() const noexcept {
         return this->_show_container_item_fields;
      }
      void setShowsContainerItemFields(bool);

      constexpr const LeveledObject* data(size_t row) const {
         if (row >= this->_items.size())
            return nullptr;
         return this->_items[row];
      }

      // Silently corrects any incorrect fields, e.g. forcing an owner of the wrong form 
      // type to None.
      void setData(size_t row, const LeveledObject&);

      constexpr bool allowsFormType(dovah::form_type ft) const noexcept {
         if (this->_allowed_form_types.empty())
            return true;
         for (auto e : this->_allowed_form_types)
            if (e == ft)
               return true;
         return false;
      }

   protected:
      std::vector<dovah::form_type> _allowed_form_types;
      std::vector<LeveledObject*> _items;
      bool _show_container_item_fields = true;

      void _clear();
};