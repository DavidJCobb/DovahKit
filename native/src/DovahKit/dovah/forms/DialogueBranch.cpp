#include "DialogueBranch.h"
#include "_common_cpp.h"
#include "../form_stub_addenda.h"

#include "../notices/form_load_warnings/by_form_type/dialogue_branch/mishandled_owning_quest_id.h"

namespace dovah::loaded_forms {
   void DialogueBranch::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      if (!intfc.is_winning_record)
         return;
      //
      Form::load(record, intfc);
      //
      form_reference_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'DNAM':
               subrecord.read(this->branch_flags);
               break;
            case 'TNAM':
               subrecord.read(this->tnam);
               break;
            case 'SNAM':
               if (subrecord.read(this->starting_topic))
                  intfc.warn_if_ref_is_wrong_type(this->starting_topic, form_type::topic, subrecord.signature());
               break;
            case 'QNAM':
               if (subrecord.read(this->owning_quest)) {
                  intfc.warn_if_ref_is_wrong_type(this->owning_quest, form_type::quest, subrecord.signature());
                  if (this->owning_quest && !subrecord.form_id_can_survive_redundant_fixup(this->owning_quest.formID())) {
                     //
                     // The game accidentally performs the local-to-global form ID conversion twice on this form ID: it 
                     // incorrectly performs the conversion in BGSDialogueBranch::LoadForm, and then correctly performs 
                     // it (with the other conversions) in BGSDialogueBranch::InitItem.
                     //
                     // When the game performs a local-to-global conversion, if the input form ID is higher than the 
                     // highest load order prefix in the file, then it is clamped to that prefix. Consider, then, the 
                     // following load order, with TestFile.esp's masters marked:
                     //
                     //    Master | L. | G. | Filename
                     //    Yes    | 00 | 00 | Skyrim.esm
                     //    Yes    | 01 | 01 | Update.esm
                     //    Yes    | 02 | 02 | Dawnguard.esm
                     //    No     | 03 |    | MyCoolFile.esp
                     //    No     | 04 |    | SomethingWeird.esp
                     //    No     | 05 |    | RandomMod.esp
                     //    Yes    | 06 | 03 | SomeFramework.esp
                     //    Self   | 07 | 04 | TestFile.esp
                     //
                     // If TestFile.esp defines a new DialogueBranch whose owning Quest is in Skyrim.esm or Update.esm, 
                     // then that reference will be interpreted correctly because those two files have the same global 
                     // load prefix as their local load prefix within TestFile.esp. However, if TestFile.esp defines a 
                     // new DialogueBranch whose owning Quest is in SomeFramework.esp, then the game will mangle the 
                     // quest's form ID when redundantly converting it: first, it will be converted from 03xxxxxx to 
                     // 06xxxxxx; then, it will be converted again. Because 06 is out-of-bounds within the local file 
                     // list (the highest local load prefix is 04), the form ID will be placed within TestFile.esp 
                     // itself, thereby being moved to 07xxxxxx.
                     //
                     // If TestFile.esp happens to have a quest with that form ID, then that quest will wrongly become 
                     // the owning quest for this DialogueBranch; otherwise, the DialogueBranch will have no owning 
                     // quest, which is also wrong. The exact impact that this has on the game is not known at this 
                     // time.
                     //
                     // DovahKit doesn't attempt to mimic this load error, because frankly, that would be insane. 
                     // However, we're obligated to warn about it.
                     //
                     // As for when this error would occur, and when it would break things? Well, most of the time, it 
                     // shouldn't actually do any damage. If you're editing dialogue in a master, you only need to 
                     // override the DLBR if you change a branch's owning quest (which I don't think the CK lets you 
                     // do, and which you have no reason to do), if you change its type or flags (again, useless), or 
                     // if you change its starting topic. If you're creating a new dialogue branch whose owning quest 
                     // belongs to Skyrim or Update, then you'll avoid the issue because the quest's form ID will 
                     // survive a redundant conversion (unless Skyrim and Update aren't the first two files in your 
                     // mod's master list, but why the hell would you ever do that?). If your dialogue branch's owning 
                     // quest belongs to the same file that supplies the branch (i.e. dialogue in your own mod), then 
                     // you'll be fine as well: the quest form ID will get mangled by the redundant conversion, but 
                     // the game will fall back to placing it within your own file.
                     //
                     // Where you're at risk is if you override a dialogue branch in another mod or in a DLC file, or 
                     // if you create a dialogue branch whose owning quest is in another mod or in a DLC file.
                     //
                     notices::form_load_warnings::by_type::dialogue_branch::mishandled_owning_quest_id notice(
                        this->stub,
                        *this->owning_quest.get_form_stub()
                     );
                     intfc.log_load_warning(notice);
                  }
               }
               break;
            case 'OBND':
               this->object_bounds.load(subrecord, intfc);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void DialogueBranch::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      //
      form_id_t starting_topic;
      form_id_t owning_quest;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'SNAM':
               subrecord.read(starting_topic);
               break;
            case 'QNAM':
               subrecord.read(owning_quest);
               break;
            case 'TNAM':
            case 'DNAM':
            case 'OBND':
               break;
         }
      }
      uib.add_outbound_reference(starting_topic);
      uib.add_outbound_reference(owning_quest, use_info::entry_flags::dialogue_branch::parent_quest);
   }
   /*virtual*/ void DialogueBranch::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (DialogueBranch*)out;
      
      bool committing_to_self = (&out->stub == &this->stub) && this->is_working_copy;
      
      copy->branch_flags = this->branch_flags;
      copy->tnam         = this->tnam;
      copy->starting_topic.set(*copy, this->starting_topic);
      copy->owning_quest.set(*copy, this->owning_quest);
      copy->object_bounds = this->object_bounds;
      copy->script_data.clone_from(this->script_data, *copy);
   }
   /*virtual*/ void DialogueBranch::_save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) {
      record.write_formID_subrecord('QNAM', this->owning_quest);
      auto& TNAM = record.open_next_subrecord('TNAM');
      TNAM.write(this->tnam);
      TNAM.close();
      auto& DNAM = record.open_next_subrecord('DNAM');
      DNAM.write(this->branch_flags);
      DNAM.close();
      record.write_formID_subrecord('SNAM', this->starting_topic);
      //
      this->script_data.save(record, intfc);
   }
   /*virtual*/ void DialogueBranch::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->starting_topic.clear_if(*this, other);
      this->owning_quest.clear_if(*this, other);
      //
      this->script_data.sever_outbound_references_to(other, *this);
   }
   /*virtual*/ void DialogueBranch::_clear_impl() noexcept {
      this->branch_flags = branch_flag::top_level;
      this->tnam         = 0;
      this->owning_quest.set(*this, nullptr);
      this->starting_topic.set(*this, nullptr);
      this->object_bounds.clear();
      this->script_data.clear(*this);
   }
}