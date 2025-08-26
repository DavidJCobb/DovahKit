#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <QAbstractItemModel>
#include <QPointer>
#include "ui/types/packages/procedure_tree_typed_data/procedure.h"
#include "./PackageDataModel.h"

class PackageProcedureParamsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      using procedure_node_type = ui::types::packages::procedure_tree_typed_data::procedure;
      struct Item {
         uint8_t unique_id = 0xFF;
         struct {
            QString name;
            QString value;
         } cached;
      };

   public:
      PackageProcedureParamsModel(QObject* parent = nullptr);

      struct Column {
         Column() = delete;
         enum {
            Name,
            Value,

            __COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::__COUNT;

      static constexpr const Qt::ItemDataRole UniqueIDRole = (Qt::ItemDataRole)(Qt::UserRole + 1);
      
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

      void clear();

      void setPackdataModel(PackageDataModel*);

      void importData(const procedure_node_type&);
      void exportData(procedure_node_type&) const;
      //
      void importData(dovah::packages::procedure_type, const std::vector<uint8_t>&);
      std::vector<uint8_t> exportData() const;

   protected:
      QPointer<PackageDataModel> _packdata_model;
      std::vector<Item> _params;

      void _pull_package_data_names(bool emit_signals);
};