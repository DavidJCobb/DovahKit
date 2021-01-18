#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include <QString>
#include <QTableView>
#include "../../../dovah/core.h"
#include "../../../dovah/forms/components/conditions.h"

namespace dovah {
   class form_stub;
}

class ConditionListModel;
class ConditionListModelItem : public dovah::loaded_forms::components::condition {
   friend ConditionListModel;
   public:
      using source_t = dovah::loaded_forms::components::condition;
      //
      void set_from(const source_t&);
      void write_to(source_t&, dovah::form_stub& owner) const;
      //
      void sever_outbound_references_to(dovah::form_stub& target) noexcept;
      QString arguments_to_string() const noexcept;
      //
   private:
      #pragma region Block access to condition methods
      bool read(dovah::tes_record_reader&, dovah::load_order_interfaces::form_load&) = delete;
      static void generate_use_info(dovah::tes_record_reader&, dovah::form_stub_use_info_builder&) = delete;
      void save(dovah::tes_record_writer&, dovah::load_order_interfaces::form_save&) = delete;
      void clone_from(const condition& original, dovah::form_stub& owner_of_clone) noexcept = delete;
      void sever_outbound_references_to(dovah::form_stub& target, dovah::form_stub& my_owner) noexcept = delete;
      //
      void to_string(std::string& out) const = delete;
      #pragma endregion
};

class ConditionListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = ConditionListModelItem;
      using form_stub = dovah::form_stub;
      //
      static constexpr int ColumnTarget   = 0;
      static constexpr int ColumnFunction = 1;
      static constexpr int ColumnArgs     = 2; // comma-separated args
      static constexpr int ColumnOperator = 3;
      static constexpr int ColumnOperand  = 4;
      static constexpr int ColumnUsesOr   = 5;
      //
   protected:
      QVector<item_type*> children;
      QVector<item_type*> queued_additions;
      //
      void insertItem(const dovah::form_stub*, bool queued);
      //
   protected slots:
      void formModified(const dovah::form_stub*);
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      void formRenumbered(const dovah::form_stub*, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID);
      //
   public slots:
      void clear();
      //
   public:
      ConditionListModel(QObject* parent = nullptr);
      ~ConditionListModel() {
         this->clear();
      }
      //
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      //
   signals:
      void columnCountChanged(); // needed so the widget can handle column sizes sensibly
};

class ConditionList : public QTableView {
   Q_OBJECT
   public:
      ConditionList(QWidget* parent);
      using model_type      = ConditionListModel;
      using model_item_type = model_type::item_type;
      //
      inline model_type* fullModel() const noexcept {
         return (model_type*)this->model();
      }
      //
   public slots:
};