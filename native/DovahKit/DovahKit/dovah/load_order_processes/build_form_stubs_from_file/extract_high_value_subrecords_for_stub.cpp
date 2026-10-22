#include "./extract_high_value_subrecords_for_stub.h"
#include <cassert>
#include "dovah/active_load_order.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_addenda.h"
#include "dovah/form_stub_addendum_types/ordered_child_collection.h"
#include "dovah/form_type.h"
#include "dovah/form_type_info.h"
#include "dovah/tes/file.h"
#include "dovah/tes/reading/record_view.h"
#include "dovah/tes/reading/subrecord_view.h"
#include "dovah/tes/record_flags.h"
#include "dovah/utils/form_stubs/get_parent_form.h"
#include "dovah/utils/form_stubs/is_exterior_cell.h"

namespace dovah::load_order_processes::build_form_stubs_from_file {
   extern void extract_high_value_subrecords_for_stub(const tes::file& source_file, form_stub& stub, tes::record_view& record) {
      //
      // This function locates "high-value"  subrecords in a record,  and uses them to acquire and 
      // store any form data that should be held on the form stub or its addenda struct. This data 
      // includes:
      //
      // 
      // ===== EDITOR ID =========================================================================
      //
      // A string (ideally unique) which identifies the form stub in development contexts, such as 
      // in the Creation Kit or when using certain console commands. We store this directly on the 
      // form stub.
      //
      //
      // ===== CELL GRID COORDINATES =============================================================
      //
      // Grid coordinates (not unit coordinates) which  indicate where in a worldspace an exterior 
      // cell should be placed. We store this in the form stub addenda.
      //
      //
      // ===== TOPIC INFO PLACEMENT ==============================================================
      //
      // Topic Infos should be placed in a special list within their parent Topics. This list must 
      // be built during load, because it is influenced by the order in which infos load. Refer to 
      // the internal  documentation <topic infos' placements in topics' info lists.txt>  for more 
      // information.
      //
      //
      const bool is_ext = utils::form_stubs::is_exterior_cell(stub);

      #pragma region INFO pre-handling
         form_stub* parent_topic = nullptr;
         if (stub.form_type == form_type::topic_info) {
            parent_topic = utils::form_stubs::get_parent_form(stub);
            if (parent_topic && parent_topic->form_type != form_type::topic)
               parent_topic = nullptr;
         }
      #pragma endregion
      
      bool       editor_id_seen        = false;
      bool       info_is_appended      = true;
      form_stub* previous_sibling_info = nullptr;

      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'EDID':
               editor_id_seen = true;
               if (!(form_type_info::lookup(stub.form_type).flags & form_type_info::flag::no_editor_id)) {
                  subrecord.read_all_to_string(stub.editor_id);
               }
               break;
            case 'PNAM':
               if (parent_topic) {
                  assert(stub.form_type == form_type::topic_info);
                  //
                  auto     pos = subrecord.position();
                  uint32_t v;
                  if (subrecord.read(v) && v != 0xFFFFFFFF) {
                     info_is_appended = false;
                     subrecord.seek(pos);

                     form_reference_t formID;
                     subrecord.read(formID);
                     previous_sibling_info = formID.get_form_stub();
                  }
               }
               break;
            case 'XCLC':
               if (is_ext) {
                  auto& pos = stub.get_or_create_addenda().grid_position.emplace();
                  subrecord.read(pos.x);
                  subrecord.read(pos.y);
               }
               break;
            default:
               continue;
         }
      }

      if (!editor_id_seen) {
         //
         // Edge-case: losing record supplies an editor ID; winning record clears it.
         //
         stub.editor_id.clear();
      }
      
      #pragma region INFO post-handling
         if (parent_topic && !stub.test_record_flags((uint32_t)1 << (size_t)tes::record_flag::deleted)) {
            auto* alo_ptr = source_file.get_active_load_order();
            assert(!!alo_ptr);
            const bool is_active_file = alo_ptr->get_active_file() == &source_file;
            if (info_is_appended) {
               parent_topic->get_or_create_addenda().ordered_children._insert_child_on_load({}, is_active_file, stub);
            } else {
               parent_topic->get_or_create_addenda().ordered_children._insert_child_on_load({}, is_active_file, stub, previous_sibling_info);
            }
         }
      #pragma endregion
   }
}