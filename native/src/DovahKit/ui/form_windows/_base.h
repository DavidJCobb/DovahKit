#pragma once
#include <cstdint>
#include <string>
#include <type_traits>
#include <QDialog>
#include "dovah/forms/components/extra_data.h"
#include "dovah/form_types.h"

namespace dovah {
   class form_stub;
}

//
// The class hierarchy and templating here is sort of janky due to the limits of QObject 
// and Qt's MOC. You can't make templated QObject classes, so we have to split a lot of 
// functionality apart in weird ways.
//
// Form editing dialogs should look like this:
//
//    class FormDialogActivator :
//       public QDialog,
//       FormEditDialogMixin<dovah::loaded_forms::Activator, true>
//    {
//       Q_OBJECT;
//       DOVAHKIT_FORM_EDIT_DIALOG;
//       public:
//          Activator(dovah::form_stub& stub, QWidget* parent = nullptr) : QDialog(parent) {
//             initialize(stub); // calls Qt's setupUi func for you
//          
//             // ... set up your widgets here -- constraints, etc ...
//          }
//
//       protected:
//          virtual void _load_impl() override {
//             // copy data from `this->form` into your UI controls here...
//          }
//          virtual void _save_impl() override {
//             // write data from your UI controls into `this->form` here...
//             // 
//             // NOTE: for dialogs that operate on a working copy, it's fine to edit 
//             //       `this->form` in real-time as your UI is interacted with
//          }
//    }
//

// Form-editing dialogs should derive from QDialog first and then from an instantiation 
// of the `FormEditDialogMixin` template below. They should not derive directly from the 
// `FormEditDialogInterface` class.
class FormEditDialogInterface {
   public:
      virtual dovah::form_type formType() const = 0;
   protected:
      virtual void _load_impl() = 0; // pull data from a loaded form into the UI
      virtual void _save_impl() = 0; // save data from the UI into a loaded form

   public:
      constexpr dovah::form_stub* formStub() const noexcept { return this->stub; }

      inline QDialog* asDialog() {
         auto* d = dynamic_cast<QDialog*>(this);
         assert(d);
         return d;
      }
      inline const QDialog* asDialog() const {
         auto* d = dynamic_cast<const QDialog*>(this);
         assert(d);
         return d;
      }

   protected:
      dovah::form_stub* stub = nullptr;
};

template<typename LoadedFormType, bool UsesWorkingCopy = false>
class FormEditDialogMixin : public FormEditDialogInterface {
   public:
      using loaded_form_type = LoadedFormType;
      using mixin_type       = FormEditDialogMixin;
      
      static constexpr const dovah::form_type form_type         = loaded_form_type::form_type;
      static constexpr const bool             uses_working_copy = UsesWorkingCopy;
   
   private:
      struct dummy_type {};
      
      struct working_copy_stub_info_type {
         std::string editor_id;
         uint32_t    record_flags = 0;
      };
      
      void _unload_form_data();
      void _load_form_data();
      
   public:
      ~FormEditDialogMixin();
      
      virtual dovah::form_type formType() const override { return form_type; }
      
   protected:
      #pragma region Accessors
      std::string& editor_id();
      
      uint32_t get_record_flags() const;
      bool test_record_flags(uint32_t bits) const;
      void edit_record_flags(uint32_t bits, bool clear_or_set);
      
      uint32_t& record_flags()
         #ifndef __INTELLISENSE__
            //
            // Spurious IntelliSense errors when subclasses try to call this member function, 
            // citing that it's unreachable because it's protected. Hiding the `requires` 
            // clause prevents this though with obvious downsides.
            //
            requires (uses_working_copy)
         #endif
      ;
      #pragma endregion
      
   protected:
      // Must be called by subclass constructor.
      void initialize(this auto&& self, dovah::form_stub&);
   
   public:
      void load(); // pull data from a loaded form into the UI
      void save(); // save data from the UI into a loaded form
      
   protected:
      // The form data to which the dialog's code should make changes.
      std::conditional_t<
         uses_working_copy,
         loaded_form_type*,
         dovah::loaded_form_ptr<loaded_form_type>
      > form = {};
      
      #pragma region Helpers
      void write_form_ref(dovah::form_reference_t&, dovah::form_stub*);
      
      template<dovah::loaded_forms::components::extra_data_type>
      void write_extra_form_ref(
         dovah::loaded_forms::components::extra_data_list&,
         dovah::form_stub*,
         bool remove_if_empty = true
      );
      #pragma endregion
      
   private:
      //
      // For dialogs that use working copies, `form` is a bare pointer to the 
      // working copy, and `_destination_form` is the "real" loaded form, to 
      // which we will commit the working copy if the user clicks "OK."
      //
      // In other words, `form` is always the thing your dialog is going to 
      // be writing to, and for working copies, `_destination_form` keeps the 
      // stub's "real" form data loaded so we can commit to it later.
      //
      std::conditional_t<
         uses_working_copy,
         dovah::loaded_form_ptr<loaded_form_type>,
         dummy_type
      > _destination_form = {};
      
      // Automate management of the editor ID and record flags for working copies.
      std::conditional_t<
         uses_working_copy,
         working_copy_stub_info_type,
         dummy_type
      > _stub_info = {};
};

#define DOVAHKIT_FORM_EDIT_DIALOG \
   friend mixin_type;

#include "./_base.inl"