#pragma once
#include <QDialog>
#include "dovah/form_stub.h"
#include "dovah/forms/components/extra_data.h"

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

class AbstractFormEditDialog;
namespace form_dialog_helpers {
   template<class Dialog> void initialize(Dialog&, dovah::form_stub*) requires std::is_base_of_v<AbstractFormEditDialog, Dialog>;
}

class AbstractFormEditDialog : public QDialog {
   Q_OBJECT;
   template<class Dialog> friend void form_dialog_helpers::initialize(Dialog&, dovah::form_stub*) requires std::is_base_of_v<AbstractFormEditDialog, Dialog>;
   public:
      AbstractFormEditDialog(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR) : QDialog(parent) {}
         
   public:
      virtual dovah::form_type_t formType() const = 0;
   protected:
      virtual void _load_impl() = 0; // pull data from a loaded form into the UI
      virtual void _save_impl() = 0; // save data from the UI into a loaded form

   public:
      constexpr const dovah::form_stub* formStub() const noexcept { return this->stub; }

   protected:
      dovah::form_stub* stub = nullptr;
};

//
// Base class for an ordinary form editing dialog. Dialogs of this type will retain 
// a user's desired changes entirely in the UI, as UI state. When the user clicks 
// "OK," the dialog's save handler (`_save_impl`) will overwrite data in the loaded 
// form with data from the dialog's UI controls.
//
class FormEditDialogBase : public AbstractFormEditDialog {
   Q_OBJECT
   private:
      using extra_data_type = dovah::loaded_forms::components::extra_data_type;
      using extra_data_list = dovah::loaded_forms::components::extra_data_list;
   public:
      FormEditDialogBase(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      
      void load();
      void save();
      
   public slots:
      virtual void accept() override;
      virtual void reject() override;
      
   protected:
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
#define DOVAHKIT_FORM_EDIT_DIALOG(LOADED_FORM_CLASS) \
   template<class Dialog> friend void form_dialog_helpers::initialize(Dialog&, dovah::form_stub*) requires std::is_base_of_v<AbstractFormEditDialog, Dialog>; \
   public: \
      using loaded_form_type = LOADED_FORM_CLASS; \
      virtual dovah::form_type_t formType() const override { return loaded_form_type::form_type; } \
   protected: \
      dovah::loaded_form_ptr<loaded_form_type> form;
;

//
// Alternate form editing dialog which relies on a "working copy" of a loaded form. 
// When the user clicks "OK," dialogs of this type will commit the working copy as 
// a whole, and use their save handler (`_save_impl`) to commit only changes that 
// the working copy would not include (i.e. data that is stored on the form stub 
// itself, such as form flags and the editor ID).
// 
// Use this base class when a form's data is too complex to be easily representable 
// through UI state alone. This base class will save you the need to mirror the full 
// contents of a form within the dialog's state.
// 
// An example of when you might use a base class like this: something like a Quest 
// form's stages, where the user can make changes to any stage, but only one stage 
// is visible at a time. You can't track the user's changes through UI state alone 
// because you have only one set of UI controls for editing any given stage. Without 
// a working copy of the Quest form, you would have to maintain copies of the stage 
// data in memory, which isn't viable because stages can refer to other forms, and 
// form references (`form_reference_t`) must be in a loaded form or working copy.
//
class FormWorkingCopyEditDialogBase : public AbstractFormEditDialog {
   Q_OBJECT
   private:
      using form_type_t = dovah::form_type_t;
      using loaded_t    = dovah::loaded_forms::Form;
   public:
      FormWorkingCopyEditDialogBase(dovah::form_type_t, dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      ~FormWorkingCopyEditDialogBase();
      
      void load();
      void save();

      virtual dovah::form_type_t formType() const override { return this->_allowed_form_type; };

      // When implementing _save_impl, write code to handle only the things that a working copy 
      // wouldn't include, like form flags and the editor ID.
      
   public slots:
      virtual void accept() override;
      virtual void reject() override;
      
   private slots:
      
   private:
      const dovah::form_type_t _allowed_form_type;
   protected:
      dovah::loaded_form_ptr<loaded_t> form;
      loaded_t* clone = nullptr; // not safe to access from _save_impl; write to the loaded form directly
      //
      template<typename C> C* get_working_copy() const noexcept { return (C*)this->clone; }
      
      virtual void _load_impl() = 0; // pull data from a loaded form into the UI
      virtual void _save_impl() = 0; // save data from the UI into a loaded form (only for things that a working copy wouldn't include, like form flags and the editor ID)
};
//
// Place this next macro inside the class definition for any FormDialogWorkingCopyBase 
// subclass, akin to the Q_OBJECT macro.
//
#define DOVAHKIT_FORM_COPY_EDIT_DIALOG(LOADED_FORM_CLASS) \
   template<class Dialog> friend void form_dialog_helpers::initialize(Dialog&, dovah::form_stub*) requires std::is_base_of_v<AbstractFormEditDialog, Dialog>; \
   public: \
      using loaded_form_type = LOADED_FORM_CLASS;
