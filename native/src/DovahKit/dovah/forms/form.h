#pragma once
#include "../core.h"

namespace dovah {
   class file_load_order;
   class form_stub;
   namespace tes_file_reading {
      class record;
   }
   namespace tes_file_writing {
      class record;
   }
   namespace load_order_interfaces {
      class form_load;
      class form_save;
   }

   namespace loaded_forms {
      class Form {
         public:
            const form_type_t formType;
            Form(form_type_t ft) : formType(ft) {};
            //
            struct form_flag {
               form_flag() = delete;
               enum : uint32_t {
                  deleted = 0x00000020, // working with this flag directly is undefined behavior. use the (flagged_as_deleted) flag on (form_stub) instead.
               };
            };
            //
            form_stub* stub  = nullptr;
            uint32_t   flags = 0;
            //
            const char* get_editor_id() const noexcept;
            void load(tes_file_reading::record& record, load_order_interfaces::form_load&);

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
            Form* clone(form_stub& receiving_stub, bool* out_complete = nullptr) const noexcept;

            bool save(tes_file_writing::record& record, load_order_interfaces::form_save& intfc); // returns a success bool. will write EDID for you.

            void friendly_delete_override(const file_load_order&) noexcept;
            void flag_as_deleted() noexcept;
            void sever_outbound_references_to(form_stub& other) noexcept;

            //
            // === void Form::setup() ============================================================
            //
            // Called on newly-created forms. Use this to set up relationships to hardcoded forms, 
            // e.g. new worldspaces using the DefaultWater water type.
            //
            virtual void setup(const file_load_order&) noexcept {}

            virtual bool would_bethesda_compress() const noexcept { return false; } // provided for CELL
            
         protected:
            virtual bool _clone_impl(Form* out) const noexcept { return false; }; // TODO: implement on existing forms; then, make pure
            virtual bool _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save&) { return false; }; // TODO: implement on existing forms; then, make pure

            //
            // === void Form::_friendly_delete_impl() ============================================
            //
            // When this function is called, perform any form-specific "friendly deletion" tasks, 
            // and return (true) if the form's "deleted" flag should NOT be set. As an example, 
            // carrying out a "friendly delete" on an ObjectReference would entail mimicking the 
            // behavior of xEdit's "Undelete and Disable Reference" function: move the reference 
            // underground, set it as an opposite enable state child of the player, and then 
            // return (true) so that we don't set the "deleted" flag.
            //
            // This function should only be called for overrides, not for forms defined in the 
            // active file.
            //
            virtual bool _friendly_delete_impl(const file_load_order&) noexcept { return false; }

            virtual void _sever_outbound_references_impl(form_stub& other) noexcept {}; // TODO: implement on existing forms; then, make pure

            static bool subrecord_is_handled_elsewhere(uint32_t signature); // call from Subclass::load
      };
   }
}