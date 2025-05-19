#pragma once
#if defined(QT_PLUGIN)
   #error This model relies on DovahKit to run. Do not include it when compiling the Qt Designer plug-in.
#endif
#include <QAbstractItemModel>
#include <QString>
#include <QVector>
#include "dovah/form_types.h"

namespace dovah {
   class form_stub;
}

class DKRefsInCellModel : public QAbstractListModel {
   Q_OBJECT
   public:
      static constexpr const Qt::ItemDataRole ForcedPrependedRole = Qt::UserRole;
      static constexpr const Qt::ItemDataRole FormStubRole        = (Qt::ItemDataRole)(Qt::UserRole + 1);

   protected:
      class Item {
         public:
            dovah::form_stub* stub = nullptr;
            struct {
               QString ref;
               QString base;
            } editor_ids;

            bool is_prepended = false;
            bool no_editor_id = false;
            QString cached_text;
            
            Item() {}
            Item(dovah::form_stub*);
            void updateFromStub(); // update the form's identifying information, e.g. its editor ID

            bool sortAbove(const Item&) const;
      };

      QVector<Item*> children;     // always kept sorted
      QVector<Item*> filtered_out; // not kept sorted
      dovah::form_stub* parent_cell = nullptr;
      dovah::form_type  filter_form_type = dovah::form_type::none;
      QString filter_string;
      std::string required_scriptname;

      bool _item_matches_filter(const Item&) const;
      bool _stub_allowed_in_model(const dovah::form_stub&) const;

      QVector<Item*> _handle_newly_concealed_by_filter();
      void _handle_newly_revealed_by_filter();

      void _clear();
      void _filter(bool filter_made_more_specific = false);
      void _insert_sorted_item(Item*);
      void _insert_sorted_stub(dovah::form_stub*, bool prepended);
      void _sort();
      
   protected slots:
      void formCreated(dovah::form_stub*);
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      void formModified(dovah::form_stub*);
      
   public:
      DKRefsInCellModel(QObject* parent = nullptr);
      ~DKRefsInCellModel();

      void addPrependedRef(dovah::form_stub*);
      void removePrependedRef(dovah::form_stub*);
      void clearAllPrependedRefs();

      dovah::form_stub* parentCell() const;
      void setParentCell(dovah::form_stub*);

      QString filterString() const;
      void setFilterString(QString);

      constexpr const std::string& requiredScriptname() const { return this->required_scriptname; }
      void setRequiredScriptname(QString);
      void setRequiredScriptname(std::string_view);

      constexpr dovah::form_type requiredFormType() const { return this->filter_form_type; }
      void setRequiredFormType(dovah::form_type);

      dovah::form_stub* ref(QModelIndex) const;
      
      #pragma region QAbstractItemModel overrides
         QModelIndex index(int row, int column, const QModelIndex& parent) const override;
         QModelIndex parent(const QModelIndex& index) const;
         int rowCount(const QModelIndex& parent) const override;
         int columnCount(const QModelIndex& item) const override;
         Qt::ItemFlags flags(const QModelIndex& index) const override;
         QVariant data(const QModelIndex& index, int role) const override;
         QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      #pragma endregion
};