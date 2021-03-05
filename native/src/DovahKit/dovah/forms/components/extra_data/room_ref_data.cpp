#include "room_ref_data.h"
#include "../../_common_cpp.h"
#include "_use_info.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result room_ref_data::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      return load_result::requires_record;
   }
   bool room_ref_data::load(tes_record_reader& record, load_interface_t& intfc) {
      {
         auto& subrecord = record.get_current_subrecord();
         assert(subrecord.signature() == signature && "This function should only have been called after the other (load) overload verified that we were in an XRMR subrecord.");
         subrecord.read(this->linked_room_count);
         subrecord.read(this->flags);
      }
      if (this->flags & flag::has_lighting_template) {
         if (record.peek_next_subrecord_type() == 'LNAM') { // the game doesn't validate the subrecord type; it'll just eat whatever comes next.
            auto& subrecord = record.next_subrecord();
            subrecord.read(this->lighting_template);
            intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
               detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::lighting_template, intfc.target_stub, this->lighting_template)
            );
         }
      }
      if (this->flags & flag::has_imagespace) {
         if (record.peek_next_subrecord_type() == 'INAM') { // the game doesn't validate the subrecord type; it'll just eat whatever comes next.
            auto& subrecord = record.next_subrecord();
            subrecord.read(this->imagespace);
            intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
               detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::imagespace, intfc.target_stub, this->imagespace)
            );
         }
      }
      if (this->linked_room_count > 0) {
         while (record.peek_next_subrecord_type() == 'XLRM') {
            //
            // NOTE: If at any point the game encounters an XRMR subrecord, it skips that subrecord 
            // but will decide to read one extra subrecord (for every XRMR it sees). Subrecord that 
            // are not XRMR are assumed to be XLRM and are blindly read as form IDs. So, to give an 
            // example, if the linked room count is five but you have two XRMR subrecords that show 
            // up before the last XLRM, then the game will try to read seven non-XRMR subrecords, 
            // all of which will be treated like XLRM.
            //
            auto& subrecord = record.next_subrecord();
            auto& formID    = this->linked_rooms.emplace_back();
            subrecord.read(formID);
            intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
               detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::reference, intfc.target_stub, formID)
            );
         }
      }
      return true;
   }
   void room_ref_data::save(tes_record_writer& record, save_interface_t& intfc) {
      this->linked_room_count = this->linked_rooms.size();
      if (this->linked_rooms.size() > std::numeric_limits<decltype(this->linked_room_count)>::max()) // check for overflow
         this->linked_room_count = std::numeric_limits<decltype(this->linked_room_count)>::max();
      //
      if (this->lighting_template)
         this->flags |= flag::has_lighting_template;
      else
         this->flags &= ~flag::has_lighting_template;
      //
      if (this->imagespace)
         this->flags |= flag::has_imagespace;
      else
         this->flags &= ~flag::has_imagespace;
      //
      auto& XRMR = record.open_next_subrecord(signature);
      XRMR.write(this->linked_room_count);
      XRMR.write(this->flags);
      XRMR.write(this->pad02);
      XRMR.close();
      if (this->lighting_template)
         record.write_formID_subrecord('LNAM', this->lighting_template);
      if (this->imagespace)
         record.write_formID_subrecord('INAM', this->imagespace);
      for (int i = 0; i < this->linked_room_count; ++i)
         record.write_formID_subrecord('XLRM', this->linked_rooms[i]);
   }
   //
   /*static*/ void room_ref_data::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
      decltype(linked_room_count) linked_room_count;
      decltype(flags) flags;
      auto& XRMR = record.get_current_subrecord();
      if (XRMR.signature() != signature) // shouldn't ever happen
         return;
      XRMR.read(linked_room_count);
      XRMR.read(flags);
      //
      form_id_t formID;
      if (flags & flag::has_lighting_template) {
         if (record.peek_next_subrecord_type() == 'LNAM') { // the game doesn't validate the subrecord type; it'll just eat whatever comes next.
            auto& subrecord = record.next_subrecord();
            subrecord.read(state.by_name.room_ref_data.lighting_template);
         }
      }
      if (flags & flag::has_imagespace) {
         if (record.peek_next_subrecord_type() == 'INAM') { // the game doesn't validate the subrecord type; it'll just eat whatever comes next.
            auto& subrecord = record.next_subrecord();
            subrecord.read(state.by_name.room_ref_data.imagespace);
         }
      }
      if (linked_room_count > 0) {
         while (record.peek_next_subrecord_type() == 'XLRM') {
            //
            // NOTE: If at any point the game encounters an XRMR subrecord, it skips that subrecord 
            // but will decide to read one extra subrecord (for every XRMR it sees). Subrecord that 
            // are not XRMR are assumed to be XLRM and are blindly read as form IDs. So, to give an 
            // example, if the linked room count is five but you have two XRMR subrecords that show 
            // up before the last XLRM, then the game will try to read seven non-XRMR subrecords, 
            // all of which will be treated like XLRM.
            //
            auto& subrecord = record.next_subrecord();
            if (subrecord.read(formID))
               uib.add_outbound_reference(formID);
         }
      }
   }
   basic_extra_data* room_ref_data::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new room_ref_data;
      clone->flags = this->flags;
      clone->pad02 = this->pad02;
      clone->lighting_template.set(clone_owner, this->lighting_template);
      clone->imagespace.set(clone_owner, this->imagespace);
      //
      size_t size = this->linked_rooms.size();
      clone->linked_rooms.resize(size);
      for (size_t i = 0; i < size; ++i)
         clone->linked_rooms[i].set(clone_owner, this->linked_rooms[i]);
      clone->linked_room_count = size;
      //
      return clone;
   }
   void room_ref_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) {
      this->lighting_template.clear_if(my_owner, target);
      this->imagespace.clear_if(my_owner, target);
      if (!this->lighting_template)
         this->flags &= ~flag::has_lighting_template;
      if (!this->imagespace)
         this->flags &= ~flag::has_imagespace;
      //
      auto& list = this->linked_rooms;
      for (auto& id : list)
         id.clear_if(my_owner, target);
      list.erase(
         std::remove_if(
            list.begin(),
            list.end(),
            [](form_reference_t& id) {
               return id == nullptr;
            }
         ),
         list.end()
      );
   }
}