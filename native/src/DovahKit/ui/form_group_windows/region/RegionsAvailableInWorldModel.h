#pragma once
#include <vector>
#include <QAbstractItemModel>
#include "ui/model_utils/self_sorting_flat_model_mixin.h"
namespace dovah {
   class form_stub;
}

class RegionsAvailableInWorldModel :
   public QAbstractItemModel,
   public ui::model_utils::self_sorting_flat_model_mixin<RegionsAvailableInWorldModel>
{
   Q_OBJECT;
   public:
      static constexpr const size_t ColumnCount = 1;

      static constexpr const Qt::ItemDataRole FormStubRole = Qt::UserRole;

   public:
      RegionsAvailableInWorldModel(QObject* parent = nullptr);

   protected:
      struct Item {
         dovah::form_stub* stub = nullptr;
         QString editor_id;
      };
      std::vector<Item> _data;
      dovah::form_stub* _worldspace = nullptr;

      static bool _sort_comparator(const Item&, const Item&);

      void _gather_regions();
      void _insert_region(dovah::form_stub&);
      void _recache(Item&);

      #pragma region Editor events
         void _on_form_created(dovah::form_stub&);
         void _on_form_modified(dovah::form_stub&);
         void _on_form_deleted(const dovah::form_stub&);
         void _on_data_acquired();
         void _on_data_abandoned();
      #pragma endregion

   public:
      void setWorldspace(dovah::form_stub*);
      QModelIndex regionIndex(const dovah::form_stub&) const;

      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent_qmi) const override;
            virtual QModelIndex parent(const QModelIndex&) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent_qmi = {}) const override;
            virtual int         columnCount(const QModelIndex& parent_qmi = {}) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex&, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex&) const override;
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion
};