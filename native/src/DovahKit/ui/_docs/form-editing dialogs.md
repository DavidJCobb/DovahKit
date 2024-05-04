
# Form-editing dialogs

There are two basic kinds of form-editing dialogs: ones that edit the target form manually, and ones that use a "working copy." The latter spawn a "working copy" which is essentially loaded form data that isn't directly tied to the stub: modifying it doesn't update Use Info, et cetera, et cetera; changes have to be committed all at once, at which point any Use Info and other changes are made.

The class hierarchy and templating for form-editing dialogs is janky due to the limits of `QObject` and Qt's MOC: you can't make templated `QObject` subclasses, so we have to split a lot of functionality apart in weird ways instead of just being able to do things in the constructor.

Dialogs should look roughly like this, using a working-copy dialog as an example:

```c++
class FormDialogActivator :
   public QDialog,
   FormEditDialogMixin<dovah::loaded_forms::Activator, true>
{
   Q_OBJECT;
   public:
      Activator(dovah::form_stub& stub, QWidget* parent = nullptr) : QDialog(parent) {
         initialize(stub); // calls Qt's setupUi func for you
      
         // set up your widgets here -- constraints, etc.

         // trigger copying form data into UI, including invocation of `_load_impl` override:
         this->load();
      }

   protected:
      virtual void _load_impl() override {
         // copy data from `this->form` into your UI controls here...
      }
      virtual void _save_impl() override {
         // write data from your UI controls into `this->form` here...
         // 
         // NOTE: for dialogs that operate on a working copy, it's fine to edit 
         //       `this->form` in real-time as your UI is interacted with
      }
}
```

Additionally, dialogs are expected to have `ui.buttonOK` and `ui.buttonCancel` widgets of type `QPushButton*`. Just place them in Qt Designer and the `initialize` helper function will do the rest.