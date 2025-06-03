#pragma once
#if defined(QT_PLUGIN)
   #error This dialog relies on DovahKit to run (dependency in DKBSACollectionModelBackend). Do not include it when compiling the Qt Designer plug-in.
#endif
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include "ui/types/conditions/condition.h"

namespace dovah {
   namespace loaded_forms {
      namespace components {
         class magic_effect_list;
      }
      class Form;
   }
   class form_stub;
}

struct DKMagicEffectListModelItem {
   dovah::form_stub* magic_effect = nullptr;
   float             magnitude    = 0.0F;
   uint32_t          area         = 0;
   uint32_t          duration     = 0;
   std::vector<ui::types::conditions::condition> conditions;

   // Cached data, to avoid doing UI-side string conversions every repaint:
   struct {
      QString  effect_name;
      QString  magic_school;
      uint32_t base_cost = 0;
   } cached;

   void _update_cache();
};

class DKMagicEffectListModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      using Item = DKMagicEffectListModelItem;

      using loaded_form  = dovah::loaded_forms::Form;
      using backend_type = dovah::loaded_forms::components::magic_effect_list;

      struct Column {
         Column() = delete;
         enum {
            Index,
            Name,
            Magnitude,
            Area,
            Duration,
            Cost,
            MagicSchool,

            _COUNT
         };
      };

   public:
      DKMagicEffectListModel(QObject* parent = nullptr);
      ~DKMagicEffectListModel();
      
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
         #pragma endregion
      #pragma endregion

      void importFrom(loaded_form& component_containing_form, const backend_type& component);
      void commitTo(loaded_form& component_containing_form, backend_type& component) const;

      constexpr bool canInsertRows(int count = 1) const noexcept {
         return this->_items.size() + count <= this->_max_effect_count;
      }
      constexpr const Item* data(size_t row) const noexcept {
         if (row >= this->_items.size())
            return nullptr;
         return this->_items[row];
      }

      void setData(size_t row, const Item&);

   protected:
      ui::types::conditions::context _condition_context;
      std::vector<Item*> _items;
      size_t _max_effect_count = 0;

      void _clear(bool signal);
};