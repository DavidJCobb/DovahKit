#include "./DKObjectReferencePicker.h"
#include <cassert>
#include <QGridLayout>
#include <QLabel>
#if !defined(QT_PLUGIN)
   #include "dovah/data/hardcoded_form_ids.h" // for PlayerRef form ID
   #include "dovah/form_stub.h"
   #include "editor/subsystems/papyrus/core.h"
   #include "editor/subsystems/worldedit/core.h"
   #include "editor/subsystems/worldedit/ref_pick_task.h"
   #include "editor/core.h"
#endif

namespace {
   constexpr const bool require_render_window_pick_hook =
      #if _DEBUG
         false
      #else
         true
      #endif
   ;
}

DKObjectReferencePicker::DKObjectReferencePicker(QWidget* parent) : QWidget(parent) {
   auto* layout = new QGridLayout(this);
   layout->setContentsMargins(0, 0, 0, 0);

   this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

   this->subwidgets.labels.cell   = new QLabel(tr("Cell:"), this);
   this->subwidgets.labels.filter = new QLabel(tr("Filter refs:"), this);
   this->subwidgets.labels.ref    = new QLabel(tr("Reference:"), this);

   {
      auto* widget = this->subwidgets.render_window_pick = new QPushButton(tr("Pick Reference in Render Window"), this);
      #if !defined(QT_PLUGIN)
         QObject::connect(widget, &QPushButton::clicked, this, [this]() {
            auto& worldedit = dovahkit::subsystems::worldedit::core::get();
            auto* task      = new dovahkit::subsystems::worldedit::ref_pick_task;
            task->cancel_on_non_matching_ref = true;
            task->ref_filter = [this](dovah::form_stub* ref) {
               if (auto& f = this->state.validation_function)
                  if (!f(ref))
                     return false;
               return ((DKRefsInCellModel*)this->subwidgets.refr->model())->refMatchesHardFilters(ref);
            };
            task->callbacks.on_complete = [this](dovah::form_stub* ref) {
               this->setRef(ref);
               this->setFocus();
               if (auto* w = this->window())
                  w->activateWindow();
            };
            task->callbacks.on_canceled = [this]() {
               this->setFocus();
               if (auto* w = this->window())
                  w->activateWindow();
            };
            worldedit.begin_pick_ref(task);
         });
      #endif
   }
   {
      auto* widget = this->subwidgets.cell = new DKFormPicker(this);
      widget->setOverrideTextForNone(tr("(any)"));
      widget->setAllowedFormType(dovah::form_type::cell);
      widget->setAllowNone(true);
      #if !defined(QT_PLUGIN)
         QObject::connect(widget, &DKFormPicker::formChanged, this, [this](dovah::form_stub* cell) {
            emit this->cellChanged(cell);
            ((DKRefsInCellModel*)this->subwidgets.refr->model())->setParentCell(cell);
         });
      #endif
   }
   {
      auto* widget = this->subwidgets.ref_filter_string = new QLineEdit(this);
      widget->setPlaceholderText("editor ID");
      #if !defined(QT_PLUGIN)
         QObject::connect(widget, &QLineEdit::textEdited, this, [this](const QString& text) {
            if (!this->state.show_ref_list_filter)
               return;
            ((DKRefsInCellModel*)this->subwidgets.refr->model())->setFilterString(text);
         });
      #endif
   }
   {
      auto* widget = this->subwidgets.refr = new DKComboBox(this);
      widget->setAutoResizeEnabled(false); // DKComboBox: force the combobox to let its contents be truncated
      #if !defined(QT_PLUGIN)
         auto* model = new DKRefsInCellModel(widget);
         widget->setModel(model);
         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            emit this->refChanged(this->ref());
         });
      #endif
   }
   {
      auto* widget = this->subwidgets.render_window_focus = new QPushButton(tr("Show Reference in Render Window"), this);
      #if !defined(QT_PLUGIN)
         QObject::connect(widget, &QPushButton::clicked, this, [this]() {
            auto* ref = this->ref();
            if (!ref)
               return;

            auto& worldedit = dovahkit::subsystems::worldedit::core::get();
            worldedit.center_on_refr(*ref);
         });
      #endif
   }

   #pragma region Tab order
   {
      this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      this->setFocusProxy(this->subwidgets.render_window_pick);
      this->setTabOrder(this->subwidgets.render_window_pick, this->subwidgets.cell);
      this->setTabOrder(this->subwidgets.cell,               this->subwidgets.ref_filter_string);
      this->setTabOrder(this->subwidgets.ref_filter_string,  this->subwidgets.refr);
      this->setTabOrder(this->subwidgets.refr,               this->subwidgets.render_window_focus);
   }
   #pragma endregion

   #if !defined(QT_PLUGIN)
   {
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, [this]() {
         auto* ref_model = ((DKRefsInCellModel*)this->subwidgets.refr->model());

         if (this->state.allow_none_ref) {
            //
            // Unloading game data clears out the DKRefsInCellModel and its prepended stubs, 
            // even a `nullptr` stub. (And even if it didn't, the model is coded so that 
            // adding the same prepended ref twice won't break anything.)
            //
            ref_model->addPrependedRef(nullptr);
         }

         if (auto* player_ref = DovahKitCore::get().get_form(dovah::hardcoded_form_ids::PlayerRef)) {
            //
            // Always prepend PlayerRef. They're not placed in any one cell, so there's no 
            // way to select them otherwise.
            //
            ref_model->addPrependedRef(player_ref);
         }
      });
      if (editor.has_data()) {
         //
         // The above only runs when we load data while the widget exists; if data is loaded 
         // already when the widget is created, we gotta fetch it here too.
         // 
         // NOTE: We need "allow none" to default to TRUE so that, when we add "None" and 
         // "PlayerRef" to the list, we always add "None" first and "None" is therefore 
         // always the default selection (rather than "PlayerRef").
         //
         auto* ref_model = ((DKRefsInCellModel*)this->subwidgets.refr->model());
         if (this->state.allow_none_ref) {
            ref_model->addPrependedRef(nullptr);
         }
         if (auto* player_ref = editor.get_form(dovah::hardcoded_form_ids::PlayerRef)) {
            ref_model->addPrependedRef(player_ref);
         }
      }
   }
   #endif

   this->_rebuildLayout();
}

//
// Even if you hide every widget in a whole row, QGridLayout can sometimes still show 
// row spacing above and below that row, effectively doubling the spacing at the gap, 
// so we have to do this terribleness.
//
void DKObjectReferencePicker::_rebuildLayout() {
   auto* layout = dynamic_cast<QGridLayout*>(this->layout());

   this->setUpdatesEnabled(false);

   layout->removeWidget(this->subwidgets.labels.cell);
   layout->removeWidget(this->subwidgets.labels.filter);
   layout->removeWidget(this->subwidgets.labels.ref);

   layout->removeWidget(this->subwidgets.render_window_pick);
   layout->removeWidget(this->subwidgets.cell);
   layout->removeWidget(this->subwidgets.ref_filter_string);
   layout->removeWidget(this->subwidgets.refr);
   layout->removeWidget(this->subwidgets.render_window_focus);

   this->subwidgets.labels.filter->setVisible(this->state.show_ref_list_filter);
   this->subwidgets.ref_filter_string->setVisible(this->state.show_ref_list_filter);
   //
   this->subwidgets.render_window_focus->setVisible(this->state.show_view_ref_button);

   int row = 0;
   {
      layout->addWidget(this->subwidgets.render_window_pick, row, 0, 1, 2);
      ++row;
   }
   {
      layout->addWidget(this->subwidgets.labels.cell, row, 0);
      layout->addWidget(this->subwidgets.cell,        row, 1);
      ++row;
   }
   if (this->state.show_ref_list_filter) {
      layout->addWidget(this->subwidgets.labels.filter,     row, 0);
      layout->addWidget(this->subwidgets.ref_filter_string, row, 1);
      ++row;
   }
   {
      layout->addWidget(this->subwidgets.labels.ref, row, 0);
      layout->addWidget(this->subwidgets.refr,       row, 1);
      ++row;
   }
   if (this->state.show_view_ref_button) {
      layout->addWidget(this->subwidgets.render_window_focus, row, 0, 1, 2);
      ++row;
   }

   this->setUpdatesEnabled(true);
}
void DKObjectReferencePicker::_updatePrependedRefs() {
   #if !defined(QT_PLUGIN)
      auto* model = ((DKRefsInCellModel*)this->subwidgets.refr->model());

      const auto& required_scriptname = model->requiredScriptname();

      bool show_none = this->state.allow_none_ref;
      if (!show_none)
         show_none = !required_scriptname.empty();

      if (show_none)
         model->addPrependedRef(nullptr);
      else
         model->removePrependedRef(nullptr);

      if (auto* player_ref = DovahKitCore::get().get_form(dovah::hardcoded_form_ids::PlayerRef)) {
         //
         // By default, we should prepend PlayerRef: they're not placed in any one cell, 
         // so there's no way to select them otherwise. However, if we're requiring a 
         // specific scriptname, then we should only prepend PlayerRef if they actually 
         // have that script attached.
         //
         bool show_player = true;
         if (!required_scriptname.empty()) {
            auto& papyrus = dovahkit::subsystems::papyrus::core::get();
            if (!papyrus.form_has_script_attached(*player_ref, required_scriptname))
               show_player = false;
         }
         if (show_player)
            model->addPrependedRef(player_ref);
         else
            model->removePrependedRef(player_ref);
      }
   #endif
}

#if !defined(QT_PLUGIN)
   dovah::form_stub* DKObjectReferencePicker::cell() const {
      return this->subwidgets.cell->formStub();
   }
   dovah::form_stub* DKObjectReferencePicker::ref() const {
      auto* widget = this->subwidgets.refr;
      return (dovah::form_stub*) widget->currentData(DKRefsInCellModel::FormStubRole).value<void*>();
   }

   void DKObjectReferencePicker::setCell(dovah::form_stub* stub) {
      if (stub) {
         if (stub->form_type != dovah::form_type::cell)
            return;
         this->subwidgets.cell->setFormStub(stub);
      } else {
         this->subwidgets.cell->setFormStub(nullptr);
      }
   }
   void DKObjectReferencePicker::setRef(dovah::form_stub* stub) {
      if (stub) {
         if (!dovah::form_type_is_reference(stub->form_type))
            return;
         if (auto& f = this->state.validation_function)
            if (!f(stub))
               return;
         auto* cell = stub->get_parent_form();
         if (cell && cell->form_type == dovah::form_type::cell) {
            this->setCell(cell);
         }
      }
      auto* widget = this->subwidgets.refr;
      auto  i      = widget->findData(QVariant::fromValue<void*>(stub), DKRefsInCellModel::FormStubRole);
      if (i >= 0)
         widget->setCurrentIndex(i);
   }

   void DKObjectReferencePicker::setValidationFunction(std::function<bool(dovah::form_stub*)>&& func) {
      this->state.validation_function = std::move(func);
      if (auto& f = this->state.validation_function)
         if (!f(this->ref()))
            this->setRef(nullptr);
   }
   void DKObjectReferencePicker::setValidationFunction(const std::function<bool(dovah::form_stub*)>& func) {
      this->state.validation_function = func;
      if (auto& f = this->state.validation_function)
         if (!f(this->ref()))
            this->setRef(nullptr);
   }
#endif

void DKObjectReferencePicker::setRefFilterString(QString filter) {
   this->state.ref_filter_string = filter;
   #if !defined(QT_PLUGIN)
      ((DKRefsInCellModel*)this->subwidgets.refr->model())->setFilterString(filter);
   #endif
}

#if !defined(QT_PLUGIN)
   const std::string& DKObjectReferencePicker::requiredScriptname() const {
      return ((DKRefsInCellModel*)this->subwidgets.refr->model())->requiredScriptname();
   }
   void DKObjectReferencePicker::setRequiredScriptname(QString desired) {
      auto* model = ((DKRefsInCellModel*)this->subwidgets.refr->model());
      model->setRequiredScriptname(desired);
      this->_updatePrependedRefs();
   }
   void DKObjectReferencePicker::setRequiredScriptname(std::string_view desired) {
      auto* model = ((DKRefsInCellModel*)this->subwidgets.refr->model());
      model->setRequiredScriptname(desired);
      this->_updatePrependedRefs();
   }
#endif

void DKObjectReferencePicker::setAllowNone(bool v) {
   auto& value = this->state.allow_none_ref;
   if (value == v)
      return;
   value = v;
   this->_updatePrependedRefs();
}
void DKObjectReferencePicker::setShowRefListFilter(bool v) {
   auto& value = this->state.show_ref_list_filter;
   if (value == v)
      return;
   value = v;
   #if !defined(QT_PLUGIN)
      if (!v) {
         //
         // Don't apply any filters if the user can't see the UI for them.
         //
         ((DKRefsInCellModel*)this->subwidgets.refr->model())->setFilterString({});
      } else {
         ((DKRefsInCellModel*)this->subwidgets.refr->model())->setFilterString(this->refFilterString());
      }
   #endif
   this->_rebuildLayout();
}
void DKObjectReferencePicker::setShowViewRefButton(bool v) {
   auto& value = this->state.show_view_ref_button;
   if (value == v)
      return;
   value = v;
   this->_rebuildLayout();
}
void DKObjectReferencePicker::setRequiredFormType(dovah::form_type ft) {
   if (ft == dovah::form_type::none)
      ft = dovah::form_type::reference;
   if (ft == this->requiredFormType())
      return;
   #if !defined(QT_PLUGIN)
   #if _DEBUG
      if (!dovah::form_type_is_reference(ft)) {
         qWarning("DKObjectReferencePicker is being told to require a form type that isn't REFR or a subclass; no forms will qualify");
      }
   #endif
   #endif
   this->state.required_form_type = ft;
   ((DKRefsInCellModel*)this->subwidgets.refr->model())->setRequiredFormType(ft);
}