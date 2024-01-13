#include "./DKObjectReferencePicker.h"
#include <cassert>
#include <QGridLayout>
#include <QLabel>
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/form_stub.h"
#endif

DKObjectReferencePicker::DKObjectReferencePicker(QWidget* parent) : QWidget(parent) {
   auto* layout = new QGridLayout(this);

   this->subwidgets.labels.cell   = new QLabel(tr("Cell:"), this);
   this->subwidgets.labels.filter = new QLabel(tr("Filter refs:"), this);
   this->subwidgets.labels.ref    = new QLabel(tr("Reference:"), this);

   {
      auto* widget = this->subwidgets.render_window_pick = new QPushButton(tr("Pick Reference in Render Window"), this);
      #if !defined(QT_DESIGNER_LIB)
         QObject::connect(widget, &QPushButton::clicked, this, [this]() {
            static_assert(false, "TODO: click handler: coordinate with Worldedit/Worldinput to pick the ref");
         });
      #endif
   }
   {
      #if !defined(QT_DESIGNER_LIB)
         auto* widget = this->subwidgets.cell = new FormPicker(this);
         widget->setAllowedFormType(dovah::form_type::cell);
         widget->setAllowNone(true); // TODO: Allow overriding the "none" text with the string "(any)"
         QObject::connect(widget, &FormPicker::formChanged, this, [this](dovah::form_stub* cell) {
            ((DKRefsInCellModel*)this->subwidgets.refr->model())->setParentCell(cell);
         });
      #else
         auto* widget = new QComboBox(this);
         widget->addItem(tr("(any)"));
      #endif
   }
   {
      auto* widget = this->subwidgets.ref_filter_string = new QLineEdit(this);
      widget->setPlaceholderText("editor ID");
      #if !defined(QT_DESIGNER_LIB)
         static_assert(false, "TODO: signal to update filter when field is edited");
      #endif
   }
   {
      auto* widget = this->subwidgets.refr = new QComboBox(this);
      #if !defined(QT_DESIGNER_LIB)
         auto* model = new DKRefsInCellModel(widget);
         widget->setModel(model);

         static_assert(false, "TODO: model for listing refs in a cell, with an optional filter");
      #endif
   }
   {
      auto* widget = this->subwidgets.render_window_focus = new QPushButton(tr("Show Reference in Render Window"), this);
      #if !defined(QT_DESIGNER_LIB)
         QObject::connect(widget, &QPushButton::clicked, this, [this]() {
            auto* ref = this->ref();
            if (!ref)
               return;
            static_assert(false, "TODO: focus ref in render window");
         });
      #endif
      layout->addWidget(widget, 2, 0, 1, 2);
   }

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

#if !defined(QT_DESIGNER_LIB)
   dovah::form_stub* DKObjectReferencePicker::cell() const {
      return this->subwidgets.cell->formStub();
   }
   dovah::form_stub* DKObjectReferencePicker::ref() const;

   void DKObjectReferencePicker::setCell(dovah::form_stub* stub) {
      if (stub) {
         assert(stub->formType == dovah::form_type::cell);
         this->subwidgets.cell->setFormStub(stub);
      } else {
         this->subwidgets.cell->setFormStub(nullptr);
      }
   }
   void DKObjectReferencePicker::setRef(dovah::form_stub* stub);
#endif

void DKObjectReferencePicker::setRefFilterString(QString filter);

void DKObjectReferencePicker::setShowRefListFilter(bool v) {
   auto& value = this->state.ref_filter_string;
   if (value == v)
      return;
   value = v;
   static_assert(false, "TODO: enable/disable filter on ref list based on whether we're showing/hiding the filter-related textbox");
   this->_rebuildLayout();
}
void DKObjectReferencePicker::setShowViewRefButton(bool v) {
   auto& value = this->state.show_view_ref_button;
   if (value == v)
      return;
   value = v;
   this->_rebuildLayout();
}