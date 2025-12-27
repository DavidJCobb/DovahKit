#include "./room_ref_data.h"
#include "../../../../_common_cpp.h"
#include "../../use_info_state.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

#include "../../../../../notices/form_load_warnings/by_form_component/extra_data/room_ref_data_insufficient_rooms.h"
#include "../../../../../notices/form_load_warnings/by_form_component/extra_data/room_ref_data_swallowed_subrecord.h"
#include "../../../../../notices/form_save_errors/by_form_component/extra_data/room_ref_data/too_many_linked_rooms.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::extra_data;
   }
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_component::extra_data::room_ref_data;
   }
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result room_ref_data::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      return subrecord_load_result::requires_record;
   }
   /*virtual*/ record_load_result room_ref_data::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      uint8_t linked_room_count;
      {
         auto& subrecord = record.get_current_subrecord();
         assert(subrecord.signature() == signature && "This function should only have been called after the other `load` overload verified that we were in an XRMR subrecord.");
         //
         // The game reads a uint32_t and manually splits it up into:
         //  - uint8_t  linked_room_count;
         //  - uint8_t  flags;
         //  - bool     is_master;
         //  - uint8_t  pad03;
         //
         subrecord.read(linked_room_count);
         subrecord.read(this->flags);
         subrecord.read(this->is_master);
         subrecord.skip_bytes(1);
      }

      if (this->flags & flag::has_lighting_template) {
         auto& subrecord = record.next_subrecord();
         if (subrecord.signature() != 'LNAM') {
            specific_load_warnings::room_ref_data_swallowed_subrecord notice(
               intfc.target_stub,
               specific_load_warnings::room_ref_data_swallowed_subrecord::expected_field_type::lighting_template,
               subrecord.signature()
            );
            intfc.log_load_warning(notice);
         }
         subrecord.read(this->lighting_template);
         intfc.warn_if_ref_is_wrong_type(this->lighting_template, form_type::lighting_template, subrecord.signature());
      }
      if (this->flags & flag::has_imagespace) {
         auto& subrecord = record.next_subrecord();
         if (subrecord.signature() != 'INAM') {
            specific_load_warnings::room_ref_data_swallowed_subrecord notice(
               intfc.target_stub,
               specific_load_warnings::room_ref_data_swallowed_subrecord::expected_field_type::imagespace,
               subrecord.signature()
            );
            intfc.log_load_warning(notice);
         }
         subrecord.read(this->imagespace);
         intfc.warn_if_ref_is_wrong_type(this->imagespace, form_type::imagespace, subrecord.signature());
      }
      if (linked_room_count > 0) {
         size_t i = 0;
         for (; i < linked_room_count; ++i) {
            auto& subrecord = record.next_subrecord();
            if (subrecord.signature() == signature) {
               //
               // What was Bethesda cooking here, and why does it smell so burnt?
               //
               --i;
               continue;
            }
            if (subrecord.signature() != 'XLRM') {
               bool would_just_be_skipped = subrecord.signature() == signature;

               specific_load_warnings::room_ref_data_swallowed_subrecord notice(
                  intfc.target_stub,
                  specific_load_warnings::room_ref_data_swallowed_subrecord::expected_field_type::linked_room,
                  subrecord.signature()
               );
               if (!would_just_be_skipped) {
                  notice.is_nth_linked_room = this->linked_rooms.size();
               }
               intfc.log_load_warning(notice);

               if (would_just_be_skipped) {
                  --i;
                  continue;
               }
            }
            auto& formID = this->linked_rooms.emplace_back();
            subrecord.read(formID);
            intfc.warn_if_ref_is_wrong_type(formID, form_type::reference, subrecord, { .nth_reference = this->linked_rooms.size() - 1 });
         }
         if (this->linked_rooms.size() < linked_room_count) {
            specific_load_warnings::room_ref_data_insufficient_rooms notice(
               intfc.target_stub,
               linked_room_count,
               this->linked_rooms.size()
            );
            intfc.log_load_warning(notice);
         }
      }
      return record_load_result::complete;
   }
   /*virtual*/ void room_ref_data::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      if (this->linked_rooms.size() >= max_linked_room_count) {
         auto notice = specific_save_errors::too_many_linked_rooms(
            *intfc.target_stub,
            this->linked_rooms.size()
         );
         intfc.throw_save_error(notice);
         return;
      }
      uint8_t linked_room_count = this->linked_rooms.size();

      if (this->lighting_template)
         this->flags |= flag::has_lighting_template;
      else
         this->flags &= ~flag::has_lighting_template;

      if (this->imagespace)
         this->flags |= flag::has_imagespace;
      else
         this->flags &= ~flag::has_imagespace;

      auto& XRMR = record.open_next_subrecord(signature);
      XRMR.write(linked_room_count);
      XRMR.write(this->flags);
      XRMR.write(this->is_master);
      XRMR.skip_bytes(1);
      XRMR.close();
      if (this->lighting_template)
         record.write_formID_subrecord('LNAM', this->lighting_template);
      if (this->imagespace)
         record.write_formID_subrecord('INAM', this->imagespace);
      for (uint8_t i = 0; i < linked_room_count; ++i)
         record.write_formID_subrecord('XLRM', this->linked_rooms[i]);
   }
   
   /*static*/ void room_ref_data::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      uint8_t         linked_room_count;
      decltype(flags) flags;
      auto& XRMR = record.get_current_subrecord();
      if (XRMR.signature() != signature) // shouldn't ever happen
         return;
      XRMR.read(linked_room_count);
      XRMR.read(flags);
      //
      form_id_t formID;
      if (flags & flag::has_lighting_template) {
         //
         // The game expects LNAM here, but never actually validates the signature.
         //
         auto& subrecord = record.next_subrecord();
         subrecord.read(uis.form_ids.by_name.room_ref_data.lighting_template);
      }
      if (flags & flag::has_imagespace) {
         //
         // The game expects INAM here, but never actually validates the signature.
         //
         auto& subrecord = record.next_subrecord();
         subrecord.read(uis.form_ids.by_name.room_ref_data.imagespace);
      }
      if (linked_room_count > 0) {
         for (size_t i = 0; i < linked_room_count; ++i) {
            auto& subrecord = record.next_subrecord();
            if (subrecord.signature() == signature) {
               //
               // What was Bethesda cooking here, and why does it smell so burnt?
               //
               --i;
               continue;
            }
            if (subrecord.read(formID))
               uib.add_outbound_reference(formID);
         }
      }
   }
   /*virtual*/ void room_ref_data::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      this->lighting_template.set(my_owner, nullptr);
      this->imagespace.set(my_owner, nullptr);
      this->flags &= ~flag::has_lighting_template;
      this->flags &= ~flag::has_imagespace;
      clear_form_reference_list(this->linked_rooms, my_owner);
   }
   /*virtual*/ void room_ref_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      this->lighting_template.clear_if(my_owner, target);
      this->imagespace.clear_if(my_owner, target);
      if (!this->lighting_template)
         this->flags &= ~flag::has_lighting_template;
      if (!this->imagespace)
         this->flags &= ~flag::has_imagespace;
      //
      remove_form_from_reference_list(this->linked_rooms, target, my_owner);
   }
   
   /*virtual*/ extra_data* room_ref_data::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new room_ref_data;
      clone->flags = this->flags;
      clone->lighting_template.set(clone_owner, this->lighting_template);
      clone->imagespace.set(clone_owner, this->imagespace);
      //
      size_t size = this->linked_rooms.size();
      clone->linked_rooms.resize(size);
      for (size_t i = 0; i < size; ++i)
         clone->linked_rooms[i].set(clone_owner, this->linked_rooms[i]);
      //
      return clone;
   }
}