#pragma once
#include <string>
#include <vector>
#include <QAbstractItemModel>
#include <QElapsedTimer>
#include <QTimer>
#include "dovah/form_types.h"

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
            QString editorID;
         };

      protected:
         shared_datastore();

         std::vector<item_type*> _forms; // always includes a "NONE" item

      protected slots:
         void _rebuild();

      public:
         static shared_datastore& get() {
            static shared_datastore instance;
            return instance;
         }

         const item_type* item_at_row(int) const noexcept;
         constexpr size_t size() const noexcept { return this->_forms.size(); }

      signals:
         void allDataCleared();
         void editorIDChanged(const item_type&, const QString& prior);
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
            in_progress,
            concluding,  // We're emitting signals.
         };

      public:
         struct filter_parameters {
            bool                      allow_none = true;
            QList<dovah::form_type> form_types;
            std::string               scriptname;
            std::string               scriptname_on_aliases;

            inline bool always_allow_none() const {
               return !scriptname.empty() || !scriptname_on_aliases.empty();
            }
         };

         static constexpr const auto FormStubRole = (Qt::ItemDataRole)(Qt::UserRole);

      protected:
         std::vector<const item_type*> _items;
         filter_parameters _last_completed_fill_params;
         struct {
            filter_parameters params;

            _fill_stage stage     = _fill_stage::inactive;
            bool        sorting   = false;
            int         progress  = 0;
            //
            QTimer timer;
            QElapsedTimer ticker;

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
         bool _entry_matches_params(const item_type&) const;

         void _refill(const filter_parameters&);
         bool _fillGrabMore(); // returns true if done
         bool _fillSortMore(); // returns true if done

      public:
         void updateParameters(const filter_parameters&);

         QString overrideTextForNone() const;
         void setOverrideTextForNone(QString);

         int indexOf(const dovah::form_stub*) const noexcept;

         constexpr bool isFilling() const noexcept { return this->_ongoing_fill.stage != _fill_stage::inactive; }

      signals:
         void beforeFilled();
         void filled();
   };
}