#pragma once
#include <QButtonGroup>
#include <QWidget>

class  ObservableStandardItemModel;
struct ObservableStandardItemModelObserver;

namespace dovahscript::impl {
   //
   // Class to traverse a QWidget hierarchy, gathering all of the widgets in that hierarchy and in 
   // any linked hierarchies. This class should only be created and used from the client thread, 
   // at the behest of the worker thread: the worker thread should send a cross-thread request to 
   // the client thread and lock until the request is acted on; the creation and execution of a 
   // hierarchy finder represents "acting on" the request.
   // 
   // Since the worker thread will be locked and waiting while the hierarchy finder runs, the 
   // worker thread will *not* be accessing the Lua state. This means that ownership of the Lua 
   // state can be temporarily transferred to the client thread, and in turn, the hierarchy 
   // finder can safely access Lua state (including to check whether a hierarchy item is Lua-
   // referenced) while running on the client thread.
   //
   class hierarchy_finder {
      public:
         struct results {
            int total_widget_count = 0; // count of all widgets in all found hierarchies
            QList<QWidget*>      root_widgets;
            QList<QButtonGroup*> button_groups;

            void append(const results&) noexcept;
            void clear() noexcept;
            bool empty() const noexcept;
            bool has_hierarchy_bridges() const noexcept;

            bool contains(QButtonGroup*) const noexcept;
            bool contains(QWidget*) const noexcept;

            // Some of the object types that we search for, like QButtonGroup, can bridge multiple 
            // widget hierarchies together. This function returns a list of all root widgets of 
            // all hierarchies that this (search_results) instance's "bridges" connect to, unless 
            // the bridges themselves or the found roots are in the (ignore) instance.
            QList<QWidget*> get_linked_hierarchies(const results& ignore) const noexcept;
         };

         QVector<ObservableStandardItemModel*> referenced_models; // models known in advance not to be abandoned

         results search_results;

         // Given some "basis" widget, traverse the widget hierarchy that that widget belongs to, 
         // checking whether any widgets or "bridge" objects in that hierarchy are Lua-referenced. 
         // This function also returns a list of the "bridges," but does not cross them to examine 
         // other hierarchies; that's the caller's responsibility.
         // 
         // Returns false if it halts the search as per (options.halt_and_clear_upon_non_abandoned).
         bool _traverse_from_basis(QWidget* basis, results& out);

         // Given a basis widget, searches the widget's entire containing hierarchy as well as any 
         // containing hierarchies linked by a QButtonGroup. Returns false if it halts the search 
         // as per (options.halt_and_clear_upon_non_abandoned).
         bool _start_from_basis(QObject*);

      public:
         void submit_non_abandoned_model(ObservableStandardItemModel*) noexcept;
         void import_non_abandoned_models(ObservableStandardItemModelObserver* exclude = nullptr) noexcept;

         void gather_from(QObject*) noexcept;

         const results& get_results() const noexcept { return this->search_results; }

         // Control whether the finder aborts, and clears its results, upon finding something that isn't 
         // abandoned. The finder will abort-and-clear by default, as a useful optimization for when the 
         // VM needs to decide what to delete.
         //
         // This option can only safely be used when running on the wrapper teardown thread.
         inline void set_stop_on_referenced(bool b) noexcept {
            this->options.halt_and_clear_upon_non_abandoned = b;
         }


   };
}