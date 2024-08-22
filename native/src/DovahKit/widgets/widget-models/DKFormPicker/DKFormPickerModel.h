#pragma once
#if defined(QT_DESIGNER_LIB)
   #error This model relies on DovahKit to run. Do not include it when compiling the Qt Designer plug-in.
#endif
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <QAbstractItemModel>
#include <QElapsedTimer>
#include <QPointer>
#include <QTimer>
#include "dovah/form_types.h"
#include "../../widget-data/DKFormPickerCustomFilter.h"

namespace dovah {
   class form_stub;
}

namespace ui::impl::DKFormPicker {
   class shared_datastore : public QObject {
      Q_OBJECT;
      public:
         struct item_type {
            dovah::form_stub* stub = nullptr;
            dovah::form_type  type = dovah::form_type::none;
            //
            bool    default_exclude_from_listings = false; // if `true`, the item is excluded from listings by default
            QString editorID;

            bool recheck_default_exclude_from_listings(); // returns true if changed
         };

      protected:
         shared_datastore();

         std::vector<item_type*> _forms; // always includes a "NONE" item

      protected slots:
         void _addForm(dovah::form_stub&); // intended for use after the initial (re)build
         void _rebuild();

      public:
         static shared_datastore& get() {
            static shared_datastore instance;
            return instance;
         }

         const item_type* item_at_row(int) const noexcept;
         const item_type* item_for_stub(const dovah::form_stub*) const; //  NOTE: probably a bit slow
         constexpr size_t size() const noexcept { return this->_forms.size(); }

      signals:
         void allDataCleared();
         void editorIDChanged(const item_type&, const QString& prior);
         void itemExclusionStateChanged(const item_type&, bool now_excluded_by_default);
         void rowsAboutToBeRemoved(size_t first, size_t last);
         void rowsInserted(size_t first, size_t last);
   };

   //
   // Gathers items from the underlying datastore and stores a list of their pointers. 
   // Items are gathered over time rather than all at once, to avoid blocking the UI; 
   // they are then sorted, also over time.
   //
   class Model : public QAbstractItemModel {
      Q_OBJECT;
      protected:
         using item_type = typename shared_datastore::item_type;

         enum class _fill_stage {
            inactive,
            currently_filling,
            currently_sorting,
            concluding,  // We're emitting signals.
         };

      public:
         using filter_function_type = std::function<bool(const dovah::form_stub*)>;

         struct filter_parameters {
            bool                    allow_none = true;
            QList<dovah::form_type> form_types;
            std::string             scriptname;
            std::string             scriptname_on_aliases;

            constexpr bool always_allow_none() const noexcept {
               return !scriptname.empty() || !scriptname_on_aliases.empty();
            }
         };

         static constexpr const auto FormStubRole = (Qt::ItemDataRole)(Qt::UserRole);

      protected:
         std::vector<const item_type*>        _items;
         std::vector<const dovah::form_stub*> _force_included_forms;
         QPointer<DKFormPickerCustomFilter> _custom_filter = nullptr;
         filter_parameters _last_completed_fill_params;
         struct {
            filter_parameters params;

            _fill_stage stage    = _fill_stage::inactive;
            int         progress = 0; // index within the list of shared_datastore items, to continue pulling items from
            //
            QTimer        timer;  // queue work to be done each tick
            QElapsedTimer ticker; // limit how much work can be done in a single tick

            std::vector<const item_type*> unsorted;
         } _ongoing_fill;
         struct {
            int  ticks_to_grab = 0;
            int  ticks_to_sort = 0;
            bool ticks_overlap = false;
         } _fill_diagnostics;
         QString _override_text_for_none;

      public:
         Model(QObject* parent);

         #pragma region QAbstractItemModel boilerplate
            #pragma region Fetching
               virtual void fetchMore(const QModelIndex& parent) override;
               virtual bool canFetchMore(const QModelIndex& parent) const override;
            #pragma endregion
            #pragma region Hierarchy
               virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
               virtual QModelIndex parent(const QModelIndex& index) const override;
               virtual int rowCount(const QModelIndex& parent) const override;
               virtual int columnCount(const QModelIndex& item) const override;
            #pragma endregion
            #pragma region Data
               virtual QVariant data(const QModelIndex& index, int role) const override;
               virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
            #pragma endregion
         #pragma endregion

      protected:
         bool _entry_matches_params(const item_type&, const filter_parameters&) const;

         decltype(_items)::iterator _insertion_point_for(const item_type&);

         void _refill(const filter_parameters&);
         void _nextFillStep();
         bool _fillGrabMore(); // returns true if done
         bool _fillSortMore(); // returns true if done
         void _finalizeFill();

         void _re_sort_item(const item_type&, std::optional<QString> prior_name, bool emit_model_sync_signals = true);

         void _on_item_exclusion_state_changed(const item_type&, bool exclude_now);

         void _force_recheck_filter(const dovah::form_stub&);

         void _force_insert_item(const item_type&, bool emit_model_sync_signals = true);
         void _force_remove_item(const item_type&, bool emit_model_sync_signals = true);

         constexpr bool _contains_item(const item_type& item) const noexcept {
            return std::find(this->_items.begin(), this->_items.end(), &item) != this->_items.end();
         }

      public:
         void updateParameters(const filter_parameters&);

         QString overrideTextForNone() const;
         void setOverrideTextForNone(QString);

         int indexOf(const dovah::form_stub*) const noexcept;

         constexpr bool isFilling() const noexcept { return this->_ongoing_fill.stage != _fill_stage::inactive; }

         bool willNeverDefaultExcludeForm(dovah::form_stub&) const;
         void setFormNeverDefaultExcluded(dovah::form_stub&, bool force_include);

         DKFormPickerCustomFilter* customFilter() const;
         void setCustomFilter(DKFormPickerCustomFilter*);

         void forceRefill();
         void forceRecheckFilterOn(const dovah::form_stub&);

      signals:
         void beforeFilled();
         void filled();
   };
}