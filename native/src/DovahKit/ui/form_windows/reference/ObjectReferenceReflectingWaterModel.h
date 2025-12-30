#pragma once
#include <vector>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
namespace dovah {
   namespace loaded_forms {
      class ObjectReference;
   }
   class form_stub;
}

class ObjectReferenceReflectingWaterModel final : public QAbstractItemModel {
   Q_OBJECT;
   public:
      ObjectReferenceReflectingWaterModel(QObject* parent = nullptr);

      enum class ExtraDataType {
         ReflectorRefs,
         LitWater,
      };

      enum class RowType {
         None,
         Reflection,
         Refraction,
         Both,
      };

      struct Column {
         Column() = delete;
         enum {
            RefName,
            RefFormID,
            Type,
         };
      };

      static constexpr const Qt::ItemDataRole FormStubRole = Qt::UserRole;
      
      using backend_form_type = dovah::loaded_forms::ObjectReference;
      
      class ReflectionTypeItemDelegate : public QStyledItemDelegate {
         Q_OBJECT;
         public:
            using QStyledItemDelegate::QStyledItemDelegate;

            QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
            void setEditorData(QWidget* editor, const QModelIndex& index) const override;
            void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
      };

   protected:
      struct Row {
         dovah::form_stub* ref = nullptr;
         RowType type = RowType::None;
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
            virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

   public:
      void import_data(ExtraDataType, backend_form_type& ref);
      void export_data(backend_form_type& ref);

      void add_reflector(dovah::form_stub&);
      void remove_reflector(dovah::form_stub&);

      bool is_valid_reflector(dovah::form_stub&) const;
      static bool is_a_reflector_at_all(dovah::form_stub&);

   protected:
      std::vector<Row> _data;
      struct {
         dovah::form_stub* ref  = nullptr;
         ExtraDataType     type = ExtraDataType::ReflectorRefs;
      } _context;

      void _on_data_abandoned();
      void _on_form_deleted(dovah::form_stub&);
      void _on_form_modified(dovah::form_stub&);
      void _on_form_renumbered(dovah::form_stub&);
      void _on_all_forms_renumbered();

      void _remove_reflection_of(dovah::form_stub&);

      decltype(_data)::iterator _insertion_point_for(const Row&);
      void _re_sort_item(size_t index);
      static bool _sort_comparator(const Row&, const Row&);

      static QString _name_of(const dovah::form_stub&);
};