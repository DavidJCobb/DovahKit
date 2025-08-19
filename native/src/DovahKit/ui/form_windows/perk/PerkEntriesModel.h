#pragma once
#include <vector>
#include <QAbstractItemModel>
#include "ui/types/perk_entries/entry.h"

namespace dovah {
   class form_stub;
}

class PerkEntriesModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      using value_type = ui::types::perk_entries::entry;

   public:
      PerkEntriesModel(QObject* parent = nullptr);

      struct Column {
         Column() = delete;
         enum {
            Rank,
            Priority,
            Type,
            Data1,
            Data2,
            Data3,
            Data4,

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

   protected:
      void _on_data_abandon_imminent();
      void _on_form_modified(dovah::form_stub*);
      void _on_form_deletion_imminent(dovah::form_stub*);

   public:
      void initializeFrom(const std::vector<value_type>&);
      void commitTo(std::vector<value_type>&) const;

      value_type item(size_t row) const;
      void setItem(size_t row, const value_type&);
      void addItem(const value_type&);
      void deleteItem(size_t row);

   protected:
      struct Entry {
         bool operator==(const Entry&) const noexcept = default; // *sigh*

         value_type data;
         std::array<QString, 4> cached_data;

         bool needs_recache_if_form_changed(const dovah::form_stub&) const;
         static bool sort(const Entry& a, const Entry& b);
      };
      std::vector<Entry> _data;

      void _insert_item(const Entry& item, bool emit_model_sync_signals);
      decltype(_data)::iterator _insertion_point_for(const Entry&);
      void _re_sort_item(const Entry&);
      void _re_sort_item(size_t row);

      void _recache_data(Entry&);
};