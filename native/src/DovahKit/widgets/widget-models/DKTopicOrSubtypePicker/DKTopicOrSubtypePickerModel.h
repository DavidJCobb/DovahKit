#pragma once
#if defined(QT_PLUGIN)
   #error This model relies on DovahKit to run. Do not include it when compiling the Qt Designer plug-in.
#endif
#include "../DKFormPicker/DKFormPickerModel.h"
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <QAbstractItemModel>
#include <QElapsedTimer>
#include <QPointer>
#include <QTimer>
#include "dovah/data/dialogue/topic_subtype.h"
#include "dovah/form_types.h"
#include "../../widget-data/DKCustomFormFilterableModelMixin.h"

namespace dovah {
   class form_stub;
}

namespace ui::impl::DKTopicOrSubtypePicker {
   using shared_datastore = ui::impl::DKFormPicker::shared_datastore;

   class Model : public QAbstractItemModel, public DKCustomFormFilterableModelMixin {
      Q_OBJECT;
      protected:
         using item_type = typename shared_datastore::item_type;
         struct cached_subtype {
            QString  name;
            uint32_t signature = 0;
         };

         enum class _fill_stage {
            inactive,
            currently_filling,
            currently_sorting,
            concluding,  // We're emitting signals.
         };

      public:
         using filter_function_type = std::function<bool(const dovah::form_stub*)>;

         struct filter_parameters {
            bool allow_none = true;
         };

         static constexpr const auto FormStubRole = (Qt::ItemDataRole)(Qt::UserRole);     // dovah::form_stub*
         static constexpr const auto SubtypeRole  = (Qt::ItemDataRole)(Qt::UserRole + 1); // uint32_t signature

      protected:
         std::vector<const item_type*>        _forms;
         std::vector<cached_subtype>          _subtypes;
         std::vector<const dovah::form_stub*> _force_included_forms;
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

         //
         // We have two sub-lists: topic subtypes, and topic forms. Additionally, we 
         // may show a "NONE" option before both lists. (Unlike DKFormPicker, we don't 
         // consider "NONE" a form. Easier to concatenate Qt's view of the lists that 
         // way.)
         //
         bool _is_showing_none_item() const;
         size_t _map_form_index_to_row(size_t i) const;
         size_t _map_row_to_form_index(size_t r) const;
         const item_type* _map_row_to_item(size_t r) const;
         const cached_subtype* _map_row_to_subtype(size_t r) const;
         bool _row_is_none(size_t r) const;

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
               
         #pragma region DKCustomFormFilterableModelMixin overrides
            virtual void recheck_custom_filter_for_all_forms() override;
            virtual void recheck_custom_filter_for_form(dovah::form_stub&) override;
         #pragma endregion

      protected:
         bool _entry_matches_params(const item_type&, const filter_parameters&) const;

         decltype(_forms)::iterator _insertion_point_for(const item_type&);

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
            return std::find(this->_forms.begin(), this->_forms.end(), &item) != this->_forms.end();
         }

      public:
         void updateParameters(const filter_parameters&);

         int rowOf(dovah::form_stub*);
         int rowOf(uint32_t subtype_signature);

         QString overrideTextForNone() const;
         void setOverrideTextForNone(QString);

         constexpr bool isFilling() const noexcept { return this->_ongoing_fill.stage != _fill_stage::inactive; }

         bool willNeverDefaultExcludeForm(dovah::form_stub&) const;
         void setFormNeverDefaultExcluded(dovah::form_stub&, bool force_include);

         void forceRefill();
         void forceRecheckFilterOn(const dovah::form_stub&);

      signals:
         void beforeFilled();
         void filled();
   };
}