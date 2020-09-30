#pragma once
#include "../core.h"

namespace dovah {
   class form_stub;
   namespace tes_file_reading {
      class record;
   }
   namespace tes_file_writing {
      class record;
   }

   namespace loaded_forms {
      class Form {
         public:
            const form_type_t formType;
            Form(form_type_t ft) : formType(ft) {};
            //
            form_stub* stub  = nullptr;
            uint32_t   flags = 0;
            //
            const char* get_editor_id() const noexcept;
            void load(tes_file_reading::record& record);

            //
            // === Form* Form::clone(form_stub&) =================================================
            // 
            // The process for creating a form is as follows: you must create a form_stub for the 
            // clone, and give it a valid file pointer and form ID. YOu must then pass that stub 
            // to the original form's (clone) method. If the cloning operation succeeds, then that 
            // method will return a newly-created loaded-form of the appropriate class; otherwise, 
            // it will return nullptr.
            //
            // If the form type contains (form_id_t) fields or similar, then use info entries may 
            // be built for your form stub during the cloning process. These will not be cleaned 
            // up if the cloning process fails; you must clear all uses yourself, the same way 
            // you would when deleting a form entirely.
            //
            // This member function links (receiving_stub) and the newly-created form together, 
            // unless the cloning process fails. If (receiving_stub) already has a non-null form 
            // pointer when passed in, an assertion will fail.
            //
            Form* clone(form_stub& receiving_stub) const noexcept;

            bool save(tes_file_writing::record& record); // returns a success bool. will write EDID for you.

            virtual bool would_bethesda_compress() const noexcept { return false; } // provided for CELL
            
         protected:
            virtual bool _clone_impl(Form* out) const noexcept { return false; }; // TODO: implement on existing forms; then, make pure
            virtual bool _save_impl(tes_file_writing::record& record) { return false; }; // TODO: implement on existing forms; then, make pure
      };
   }
}