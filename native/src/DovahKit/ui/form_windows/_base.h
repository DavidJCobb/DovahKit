#pragma once
#include <QDialog>
#include "../../dovah/form_stub.h"
#include "../../dovah/forms/components/extra_data.h"

/*

   FORM-EDITING DIALOGS

   These dialogs are the principal means through which users can edit forms' 
   data. Each dialog should subclass FormDialogBaseTemplate, below, and should 
   use the DOVAHKIT_FORM_EDIT_DIALOG macro on the line after the Q_OBJECT 
   macro. Additionally, the subclass constructors should begin with a call to 
   (form_dialog_helpers::initialize), and SHOULD NOT call (setupUi) on their 
   UI objects, as the initialize function does that for them.

   Dialogs must offer two QPushButtons named "buttonOK" and "buttonCancel", 
   respectively. They must also have a loaded_form_ptr member named (form).

   The goal of this arrangement is to keep the amount of boilerplate for each 
   form-editing dialog to a minimum. All behaviors and functionality common 
   to all form-editing dialogs should be handled by the superclass itself or 
   by the templated (form_dialog_helpers::initialize) function.

   As for the *reason* for this arrangement: QObject classes cannot be 
   templated, as that confuses Qt's MOC tools; thus the external function.

*/

namespace form_dialog_helpers {
   template<class _dialog_t, typename loaded_form_t> void initialize(_dialog_t& dialog, dovah::form_stub* stub);
}

class FormDialogBaseTemplate : public QDialog {
   Q_OBJECT
   private:
      using extra_data_type = dovah::loaded_forms::components::extra_data_type;
      using extra_data_list = dovah::loaded_forms::components::extra_data_list;
   public:
      FormDialogBaseTemplate(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      //
      void load();
      void save();
      //
   private slots:
      //
   protected:
      dovah::form_stub* stub = nullptr;
      //
      virtual void _load_impl() = 0; // pull data from a loaded form into the UI
      virtual void _save_impl() = 0; // save data from the UI into a loaded form
      //
      void save_form_id(dovah::form_reference_t& target, dovah::bare_form_id_t);
      void save_form_id(dovah::form_reference_t& target, dovah::form_stub*);
      
      // helper/shortcut function ONLY suitable for instances of formID_extra_data
      void save_extra_form(dovah::bare_form_id_t, extra_data_list&, extra_data_type, bool remove_if_no_form = true);
      // helper/shortcut function ONLY suitable for instances of formID_extra_data
      void save_extra_form(dovah::form_stub*,     extra_data_list&, extra_data_type, bool remove_if_no_form = true);
};
#define DOVAHKIT_FORM_EDIT_DIALOG template<class _dialog_t, typename loaded_form_t> friend void form_dialog_helpers::initialize(_dialog_t& dialog, dovah::form_stub* stub);