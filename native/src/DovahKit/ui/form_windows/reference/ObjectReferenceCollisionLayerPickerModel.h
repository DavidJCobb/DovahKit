#pragma once
#include <vector>
#include <QAbstractItemModel>
namespace dovah {
   class form_stub;
}
namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   class collision_layer;
}

class ObjectReferenceCollisionLayerPickerModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      ObjectReferenceCollisionLayerPickerModel(QObject* parent = nullptr);

      static constexpr const size_t ColumnCount = 1;

   protected:
      struct Item {
         dovah::form_stub* form      = nullptr;
         uint32_t          layer_uid = 0;
         QString           name;
      };

   public:
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

      void setIsForTriggerVolume(bool);

   protected:
      std::vector<Item> _data;
      bool _is_for_trigger_volume = true;

      bool _is_valid(const dovahkit::subsystems::form_info_cache::cached_data::by_form::collision_layer&);
      size_t _index_of(const dovah::form_stub&) const;

      static bool _compare_for_sort(const Item&, const Item&);
      int _insert_item(const Item& item, bool emit_model_sync_signals); // returns row index
      decltype(_data)::iterator _insertion_point_for(const Item&);
      void _re_sort_item(size_t row);

      void _gather_all_layers();
      void _on_data_abandoned();
      void _on_form_deleted(dovah::form_stub&);
      void _on_form_modified(dovah::form_stub&);

      void _insert_form_if_valid(dovah::form_stub&, bool emit_signals);
};