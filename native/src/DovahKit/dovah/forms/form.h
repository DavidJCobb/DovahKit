#pragma once
#include "dovah/core.h"

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
   namespace loaded_forms::components {
      namespace papyrus {
         class attachment_data;
      }
      class  model;
      struct object_bounds;
      using  papyrus_attachment_data = papyrus::attachment_data;
   }

   namespace loaded_forms {
      // Sentinel base class for any forms that aren't fully implemented. This class will prevent DovahKit 
      // from attempting to construct any such forms while running; it's used as a sentinel for template 
      // metaprogramming. There are also constexpr bools that can be flipped to trigger static assertion 
      // failures if any forms inherit from this type, as a way to quickly check if any forms are incomplete 
      // before shipping a build.
      // 
      // The forms in question should always derive from the base Form type first, and then from this.
      struct _IncompleteFormType {};

      class Form {
         friend class form_stub;
         public:
            static constexpr const form_type form_type = form_type::none;
            //
            const enum form_type type;
            const bool is_working_copy;
            form_stub& stub;
            
            struct form_flag {
               form_flag() = delete;
               enum : uint32_t {
                  deleted = 0x00000020, // working with this flag directly is undefined behavior. use the (flagged_as_deleted) flag on (form_stub) instead.
               };
            };

            struct constructor_params {
               bool       is_working_copy = false;
               form_stub* stub            = nullptr; // MUST not be nullptr, but made a pointer to allow flexibility in when to set it
            };
            //
            Form(enum form_type ft, const constructor_params&);
            
            const char* get_editor_id() const noexcept;
            void load(tes_file_reading::record& record, load_order_interfaces::form_load&);

            const components::papyrus_attachment_data* get_raw_papyrus_data() const noexcept;
            components::papyrus_attachment_data* get_raw_papyrus_data() noexcept;
            components::object_bounds* get_object_bounds() noexcept;
            components::model* get_model() noexcept;

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

            void clear(); // clear absolutely all data on the form

            void save(tes_file_writing::record& record, load_order_interfaces::form_save& intfc); // throws on failure. will write EDID for you.

            void friendly_delete_override(const file_load_order&) noexcept;
            void flag_as_deleted() noexcept;
            void sever_outbound_references_to(form_stub& other) noexcept; // if (this) is not a working copy and (this->stub) has a working copy, calls the same function on the working copy as well

            //
            // === void Form::setup() ============================================================
            //
            // Called on newly-created forms. Use this to set up relationships to hardcoded forms, 
            // e.g. new worldspaces using the DefaultWater water type.
            //
            virtual void setup(const file_load_order&) noexcept {}

            virtual bool would_bethesda_compress() const noexcept { return false; } // provided for CELL
            
         protected:
            virtual void _clone_impl(Form* out) const noexcept {}; // TODO: implement on existing forms; then, make pure
            virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save&);
            virtual void _clear_impl() noexcept {}; // TODO: implement on existing forms; then, make pure

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