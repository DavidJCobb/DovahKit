#pragma once
#include <vector>
#include <QAbstractItemModel>
#include "dovah/forms/ImpactDataSet.h"

class ImpactDataSetContentsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      ImpactDataSetContentsModel(QObject* parent = nullptr);

      struct Column {
         Column() = delete;
         enum {
            MaterialTypeName,
            MaterialTypeFormID,
            ImpactDataName,
            ImpactDataFormID,
         };
      };
      static constexpr const size_t ColumnCount = 4;

      static constexpr const Qt::ItemDataRole FormStubRole = Qt::UserRole;
      
      using backend_form_type = dovah::loaded_forms::ImpactDataSet;
      using backend_pair_type = backend_form_type::mapping;

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
      void importData(const backend_form_type&);
      void exportData(backend_form_type&) const;

      QModelIndex indexOfMaterial(const dovah::form_stub&);
      void setImpactData(const dovah::form_stub&, dovah::form_stub*);
      void setImpactData(QModelIndex material_qmi, dovah::form_stub*);

      void setAllUnsetTo(dovah::form_stub* impact_data);

   protected:
      std::vector<std::pair<dovah::form_stub*, dovah::form_stub*>> _data;

      void _on_data_acquired();
      void _on_data_abandoned();
      void _on_form_created(dovah::form_stub&);
      void _on_form_deleted(dovah::form_stub&);
      void _on_form_modified(dovah::form_stub&);
      void _on_form_renumbered(dovah::form_stub&);
      void _on_all_forms_renumbered();

      decltype(_data)::iterator _insertion_point_for(QString material_type_editor_id);
      void _re_sort_item(size_t index);
};