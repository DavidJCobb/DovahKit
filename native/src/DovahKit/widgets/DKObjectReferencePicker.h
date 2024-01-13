#pragma once
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#if !defined(QT_DESIGNER_LIB)
   #include "../dovah/core.h"
   #include "./widget-models/DKRefsInCellModel.h"
#endif

namespace dovah {
   class form_stub;
}
#if !defined(QT_DESIGNER_LIB)
   #include "ui/generic/FormPicker.h"
#else
   class FormPicker;
#endif

class DKObjectReferencePicker : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(bool showRefListFilter READ showRefListFilter WRITE setShowRefListFilter DESIGNABLE true);
   Q_PROPERTY(bool showViewRefButton READ showViewRefButton WRITE setShowViewRefButton DESIGNABLE true);
   public:
      DKObjectReferencePicker(QWidget* parent);

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

      void setShowRefListFilter(bool);
      void setShowViewRefButton(bool);

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

         FormPicker*  cell = nullptr;
         QComboBox*   refr = nullptr;
      } subwidgets;
      struct {
         bool show_ref_list_filter = false;
         bool show_view_ref_button = false;

         QString ref_filter_string;
      } state;

      void _rebuildLayout();
};