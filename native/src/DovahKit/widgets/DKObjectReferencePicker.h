#pragma once
#include <string_view>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "dovah/form_types.h"
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/core.h"
   #include "./widget-models/DKRefsInCellModel.h"
#endif
#include "./DKFormPicker.h"

namespace dovah {
   class form_stub;
}

class DKObjectReferencePicker : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(bool allowNone         READ allowNone         WRITE setAllowNone DESIGNABLE true);
   Q_PROPERTY(dovah::form_type requiredFormType READ requiredFormType WRITE setRequiredFormType DESIGNABLE true);
   Q_PROPERTY(bool showRefListFilter READ showRefListFilter WRITE setShowRefListFilter DESIGNABLE true);
   Q_PROPERTY(bool showViewRefButton READ showViewRefButton WRITE setShowViewRefButton DESIGNABLE true);
   public:
      DKObjectReferencePicker(QWidget* parent);

      inline bool allowNone() const noexcept { return this->state.allow_none_ref; }
      inline bool showRefListFilter() const noexcept { return this->state.show_ref_list_filter; }
      inline bool showViewRefButton() const noexcept { return this->state.show_view_ref_button; }

      #if !defined(QT_DESIGNER_LIB)
         dovah::form_stub* cell() const;
         dovah::form_stub* ref() const;
      #endif

      inline QString refFilterString() const noexcept { return this->state.ref_filter_string; }

   public slots:
      #if !defined(QT_DESIGNER_LIB)
         void setCell(dovah::form_stub*);
         void setRef(dovah::form_stub*);
      #endif

      void setRefFilterString(QString);

      #if !defined(QT_DESIGNER_LIB)
         // NOTE: This forces "allow none" when used. This is required in order to allow for the 
         //       edge-case of a cell having refs, but none with the desired scriptname.
         const std::string& requiredScriptname() const;
         void setRequiredScriptname(QString);
         void setRequiredScriptname(std::string_view);
      #endif

      void setAllowNone(bool);
      void setShowRefListFilter(bool);
      void setShowViewRefButton(bool);

      constexpr dovah::form_type requiredFormType() const { return this->state.required_form_type; }
      void setRequiredFormType(dovah::form_type);

   signals:
      void cellChanged(dovah::form_stub*);
      void refChanged(dovah::form_stub*);

   protected:
      struct {
         struct {
            QLabel* cell   = nullptr;
            QLabel* filter = nullptr;
            QLabel* ref    = nullptr;
         } labels;

         QPushButton* render_window_pick  = nullptr;
         QPushButton* render_window_focus = nullptr;
         QLineEdit*   ref_filter_string   = nullptr;

         DKFormPicker* cell = nullptr;
         QComboBox*    refr = nullptr;
      } subwidgets;
      struct {
         bool allow_none_ref       = true;
         bool show_ref_list_filter = false;
         bool show_view_ref_button = false;

         QString          ref_filter_string;
         dovah::form_type required_form_type = dovah::form_type::reference;
      } state;

      void _rebuildLayout();
      void _updatePrependedRefs();
};