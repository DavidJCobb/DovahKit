#pragma once

namespace editor_helpers::condition_to_string::options {
   enum class editor_id_presence {
      never,
      only_for_form,
      form_or_base,
   };
   enum class form_id_presence {
      never,
      always,
      if_no_editor_id,
   };
   enum class form_type_format {
      //         // EDITOR ID AND FORM ID         / EDITOR ID ONLY     / FORM ID ONLY
      //         // ==============================/====================/==================
      none,      // "EditorID (00123456)"         / "EditorID"         / "(00123456)"
      name,      // "Form: 'EditorID' (00123456)" / "Form: 'EditorID'" / "Form (00123456)"
      signature, // "[FORM:00123456]EditorID"     / "[FORM]EditorID"   / "[FORM:00123456]"
   };
   struct form_format {
      form_type_format   form_type         : 2 = form_type_format::none;
      editor_id_presence include_editor_id : 2 = editor_id_presence::only_for_form;
      form_id_presence   include_form_id   : 2 = form_id_presence::if_no_editor_id;
      form_id_presence   include_placement : 2 = form_id_presence::never;
   };
}