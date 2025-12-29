#pragma once
#include <vector>
#include <QAbstractItemModel>
namespace dovah {
   namespace loaded_forms {
      namespace components::extra_data_types {
         class activate_parents;
      }
      class ObjectReference;
   }
   class form_stub;
}

class ObjectReferenceActivateParentsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      ObjectReferenceActivateParentsModel(QObject* parent = nullptr);

      struct Column {
         Column() = delete;
         enum {
            RefName,
            RefFormID,
            Delay,
         };
      };
      static constexpr const size_t ColumnCount = 3;

      static constexpr const Qt::ItemDataRole FormStubRole = Qt::UserRole;
      static constexpr const Qt::ItemDataRole DelayRole    = (Qt::ItemDataRole)(Qt::UserRole + 1);
      
      using backend_form_type = dovah::loaded_forms::ObjectReference;
      using extra_data_type   = dovah::loaded_forms::components::extra_data_types::activate_parents;

   protected:
      struct Mapping {
         dovah::form_stub* ref = nullptr;
         float delay = 0;
         struct {
            QString keyword;
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
         #pragma region Editing
            virtual bool removeRows(int row, int count, const QModelIndex& parent = {}) override;
         #pragma endregion
      #pragma endregion

   public:
      void importData(const backend_form_type&);
      void exportData(backend_form_type&) const;

      QModelIndex setRefDelay(dovah::form_stub&, float);
      void setRow(size_t i, dovah::form_stub& ref, float delay);

      [[nodiscard]] std::vector<dovah::form_stub*> allRefs() const;
      bool containsRef(const dovah::form_stub&) const;

   protected:
      std::vector<Mapping> _data;

      void _on_data_abandoned();
      void _on_form_deleted(dovah::form_stub&);
      void _on_form_modified(dovah::form_stub&);
      void _on_form_renumbered(dovah::form_stub&);
      void _on_all_forms_renumbered();

      static QString _name_of(const dovah::form_stub&);
};