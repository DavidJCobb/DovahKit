#include "./idle_collection.h"
#include "../_common_cpp.h"

#include "../../notices/form_load_warnings/by_form_component/idle_collection/incorrect_idle_count.h"
#include "../../notices/form_save_errors/by_form_component/idle_collection/too_many_idles.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::idle_collection;
   }
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_component::idle_collection;
   }

   enum class save_behavior {
      // Don't save anything if the idle list is empty.
      vanilla,

      always_save_non_zero,

      always_save_all,
   };
   constexpr const auto use_save_behavior = save_behavior::always_save_non_zero;
}

namespace dovah::loaded_forms::components {
   void idle_collection::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      auto signature = subrecord.signature();
      switch (signature) {
         case subrecord_signature_array:
            if (subrecord.size() != this->_load_state.count * 4) {
               specific_load_warnings::incorrect_idle_count notice(
                  intfc.target_stub,
                  this->_load_state.count,
                  subrecord.size() / 4
               );
               intfc.log_load_warning(notice);
               break;
            }
            this->idles.reserve(this->_load_state.count);
            for (uint8_t i = 0; i < this->_load_state.count; ++i) {
               auto& form = this->idles.emplace_back();
               if (!subrecord.read(form)) {
                  this->idles.pop_back();
                  break;
               }
               intfc.warn_if_ref_is_wrong_type(form, form_type::idle, subrecord.signature());
            }
            break;
         case subrecord_signature_count:
            subrecord.read(this->_load_state.count);
            break;
         case subrecord_signature_flags:
            subrecord.read(this->flags);
            break;
         case subrecord_signature_timer:
            subrecord.read(this->timer);
            break;

         default: // invalid
            assert(false && "Why was idle_collection::load called on a subrecord it's not built to handle?");
      }
   }
   void idle_collection::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      if constexpr (use_save_behavior == save_behavior::vanilla) {
         if (this->idles.empty())
            return;
      }
      bool save_flags = true;
      bool save_count = true;
      bool save_timer = true;
      if constexpr (use_save_behavior == save_behavior::always_save_non_zero) {
         if (this->idles.empty()) {
            save_flags = this->flags != 0;
            save_count = false;
            save_timer = this->timer != 0;
         }
      }

      if (save_flags) {
         auto& subrecord = record.open_next_subrecord('IDLF');
         subrecord.write(this->flags);
         subrecord.close();
      }
      if (save_count) {
         const size_t size = this->idles.size();
         if (size > max_idles_count) {
            auto notice = specific_save_errors::too_many_idles(
               *intfc.target_stub,
               size
            );
            intfc.throw_save_error(notice);
         }
         auto& subrecord = record.open_next_subrecord('IDLC');
         subrecord.write((uint8_t)size);
         subrecord.close();
      }
      if (save_timer) {
         auto& subrecord = record.open_next_subrecord('IDLT');
         subrecord.write(this->timer);
         subrecord.close();
      }
      if (!this->idles.empty()) {
         auto& subrecord = record.open_next_subrecord('IDLA');
         for (auto& idle : this->idles)
            subrecord.write(idle);
         subrecord.close();
      }
   }
   void idle_collection::clone_from(const idle_collection& original, loaded_forms::Form& owner_of_clone) noexcept {
      copy_form_reference_list(owner_of_clone, this->idles, original.idles);
      this->timer = original.timer;
      this->flags = original.flags;
   }
   void idle_collection::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      remove_form_from_reference_list(this->idles, target, my_owner);
   }
   void idle_collection::clear(loaded_forms::Form& my_owner) {
      clear_form_reference_list(this->idles, my_owner);
      this->timer = 0;
      this->flags = 0;
   }

   void idle_collection::use_info_state::read(tes_subrecord_reader& subrecord) {
      auto signature = subrecord.signature();
      switch (signature) {
         case subrecord_signature_array:
            if (subrecord.size() != this->count * 4) {
               break;
            }
            this->idles.resize(this->count);
            for (uint8_t i = 0; i < this->count; ++i) {
               auto& form = this->idles.emplace_back();
               if (!subrecord.read(form))
                  break;
            }
            break;
         case subrecord_signature_count:
            subrecord.read(this->count);
            break;

         case subrecord_signature_flags:
         case subrecord_signature_timer:
            break;

         default: // invalid
            assert(false && "Why was idle_collection::load called on a subrecord it's not built to handle?");
      }
   }
   void idle_collection::use_info_state::commit(form_stub_use_info_builder& uib) {
      for(auto id : this->idles)
         uib.add_outbound_reference(id);
   }
}