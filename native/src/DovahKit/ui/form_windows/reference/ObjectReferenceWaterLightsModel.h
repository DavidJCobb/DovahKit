#pragma once
#include <vector>
#include <QAbstractItemModel>
namespace dovah {
   namespace loaded_forms {
      namespace components::extra_data_types {
         class lit_water;
      }
      class ObjectReference;
   }
   class form_stub;
}

class ObjectReferenceWaterLightsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      ObjectReferenceWaterLightsModel(QObject* parent = nullptr);

      struct Column {
         Column() = delete;
         enum {
            RefName,
            RefFormID,
         };
      };
      static constexpr const size_t ColumnCount = 2;

      static constexpr const Qt::ItemDataRole FormStubRole = Qt::UserRole;
      
      using backend_form_type = dovah::loaded_forms::ObjectReference;
      using extra_data_type   = dovah::loaded_forms::components::extra_data_types::lit_water;

   protected:
      struct Row {
         dovah::form_stub* ref = nullptr;
         struct {
            QString ref;
         } cached;
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

   public:
      void setSubject(dovah::form_stub* ref);

   protected:
      std::vector<Row> _data;
      dovah::form_stub* _subject = nullptr;

      void _on_data_abandoned();
      void _on_form_deleted(dovah::form_stub&);
      void _on_form_modified(dovah::form_stub&);
      void _on_form_renumbered(dovah::form_stub&);
      void _on_all_forms_renumbered();

      void _remove_ref(dovah::form_stub&);
      void _update_ref(dovah::form_stub&);

      decltype(_data)::iterator _insertion_point_for(const Row&);
      void _re_sort_item(size_t index);
      static bool _sort_comparator(const Row&, const Row&);

      static QString _name_of(const dovah::form_stub&);
};