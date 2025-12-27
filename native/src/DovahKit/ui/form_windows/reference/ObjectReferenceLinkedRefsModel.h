#pragma once
#include <vector>
#include <QAbstractItemModel>
namespace dovah {
   namespace loaded_forms {
      namespace components::extra_data_types {
         class linked_ref;
      }
      class ObjectReference;
   }
   class form_stub;
}

class ObjectReferenceLinkedRefsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      ObjectReferenceLinkedRefsModel(QObject* parent = nullptr);

      struct Column {
         Column() = delete;
         enum {
            KeywordName,
            RefName,
            RefFormID,
         };
      };
      static constexpr const size_t ColumnCount = 3;

      static constexpr const Qt::ItemDataRole FormStubRole = Qt::UserRole;
      
      using backend_form_type = dovah::loaded_forms::ObjectReference;
      using extra_data_type   = dovah::loaded_forms::components::extra_data_types::linked_ref;

   protected:
      struct Mapping {
         dovah::form_stub* keyword = nullptr;
         dovah::form_stub* ref     = nullptr;
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
      #pragma endregion

   public:
      void importData(const backend_form_type&);
      void exportData(backend_form_type&) const;

      QModelIndex index(const dovah::form_stub& keyword) const;
      void setLink(dovah::form_stub* keyword, dovah::form_stub* refr);

      // Includes nullptr, if there exists a keywordless linked ref.
      [[nodiscard]] std::vector<dovah::form_stub*> allKeywords() const;

   protected:
      std::vector<Mapping> _data;

      void _on_data_abandoned();
      void _on_form_deleted(dovah::form_stub&);
      void _on_form_modified(dovah::form_stub&);
      void _on_form_renumbered(dovah::form_stub&);
      void _on_all_forms_renumbered();

      decltype(_data)::iterator _insertion_point_for(const Mapping&);
      void _re_sort_item(size_t index);

      static QString _name_of(const dovah::form_stub&);
};