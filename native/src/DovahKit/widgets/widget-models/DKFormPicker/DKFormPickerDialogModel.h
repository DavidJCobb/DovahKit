#pragma once
#include <QAbstractItemModel>
#include <QList>
#include <optional>
#include <vector>
#include "./DKFormPickerModel.h"
#include "dovah/form_types.h"

namespace ui::impl::DKFormPicker {
   class DialogModel : public QAbstractItemModel, public DKCustomFormFilterableModelMixin {
      Q_OBJECT;
      protected:
         using item_type = typename shared_datastore::item_type;

      public:
         static constexpr const auto FormStubRole = (Qt::ItemDataRole)(Qt::UserRole);

      protected:
         QList<dovah::form_type>       _allowed_form_types;
         std::vector<const item_type*> _items;
         QString _filter;

         bool _updates_enabled = true;
         bool _updates_pending = false;

      public:
         DialogModel(QObject* parent);

         #pragma region QAbstractItemModel boilerplate
            #pragma region Hierarchy
               virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
               virtual QModelIndex parent(const QModelIndex& index) const override;
               virtual int rowCount(const QModelIndex& parent) const override;
               virtual int columnCount(const QModelIndex& item) const override;
            #pragma endregion
            #pragma region Data
               virtual QVariant data(const QModelIndex& index, int role) const override;
               virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
               virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
            #pragma endregion
         #pragma endregion
               
         #pragma region DKCustomFormFilterableModelMixin overrides
            virtual void recheck_custom_filter_for_all_forms() override;
            virtual void recheck_custom_filter_for_form(dovah::form_stub&) override;
         #pragma endregion

      protected:
         bool _entry_matches_params(const item_type&) const;
         bool _entry_matches_filter_string(const item_type&) const;

         decltype(_items)::iterator _insertion_point_for(const item_type&);

         void _refill();
         void _tighten_filter();

         void _re_sort_item(const item_type&, std::optional<QString> prior_name);

         void _on_item_exclusion_state_changed(const item_type&, bool exclude_now);

         void _force_insert_item(const item_type&, bool emit_model_sync_signals = true);
         void _force_remove_item(const item_type&, bool emit_model_sync_signals = true);

         constexpr bool _contains_item(const item_type& item) const noexcept {
            return std::find(this->_items.begin(), this->_items.end(), &item) != this->_items.end();
         }

      public:
         constexpr const QList<dovah::form_type>& allowedFormTypes() const noexcept { return this->_allowed_form_types; }
         void setAllowedFormTypes(const QList<dovah::form_type>&);

         inline QString filterString() const noexcept { return this->_filter; }
         void setFilterString(QString);

         constexpr bool updatesEnabled() const noexcept { return this->_updates_enabled; }
         void setUpdatesEnabled(bool);

         QModelIndex index(const dovah::form_stub*) const;
   };
}