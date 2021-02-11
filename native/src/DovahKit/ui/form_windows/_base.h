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
   template<class _dialog_t> void initialize(_dialog_t& dialog);
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
      inline const dovah::form_stub* formStub() const noexcept { return this->stub; }
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
//
// Place this next macro inside the class definition for any FormDialogBaseTemplate 
// subclass, akin to the Q_OBJECT macro.
//
#define DOVAHKIT_FORM_EDIT_DIALOG template<class _dialog_t, typename loaded_form_t> friend void form_dialog_helpers::initialize(_dialog_t& dialog, dovah::form_stub* stub);

class FormDialogWorkingCopyBase : public QDialog {
   Q_OBJECT
   private:
      using form_type_t = dovah::form_type_t;
      using loaded_t    = dovah::loaded_forms::Form;
   public:
      FormDialogWorkingCopyBase(dovah::form_type_t, dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      ~FormDialogWorkingCopyBase();
      //
      void load();
      void save();
      //
      inline const dovah::form_stub* formStub() const noexcept { return this->stub; }
      //
   public slots:
      virtual void accept() override;
      virtual void reject() override;
      //
   private slots:
      //
   private:
      const dovah::form_type_t _allowed_form_type;
   protected:
      dovah::form_stub* stub = nullptr;
      dovah::loaded_form_ptr<loaded_t> form;
      loaded_t* clone = nullptr; // not safe to access from _save_impl; write to the loaded form directly
      //
      template<typename C> C* get_working_copy() const noexcept { return (C*)this->clone; }
      //
      virtual void _load_impl() = 0; // pull data from a loaded form into the UI
      virtual void _save_impl() = 0; // save data from the UI into a loaded form (only for things that a working copy wouldn't include, like form flags and the editor ID)
};
//
// Place this next macro inside the class definition for any FormDialogWorkingCopyBase 
// subclass, akin to the Q_OBJECT macro.
//
#define DOVAHKIT_FORM_COPY_EDIT_DIALOG template<class _dialog_t> friend void form_dialog_helpers::initialize(_dialog_t& dialog);
