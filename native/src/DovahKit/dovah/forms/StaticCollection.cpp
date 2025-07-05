#include "StaticCollection.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/static_collection/orphaned_instances.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::static_collection;
   }
}

namespace dovah::loaded_forms {
   void StaticCollection::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      size_t orphaned_instances_count = 0;
      form_reference_t last_seen_static;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;

            case 'ONAM':
               if (subrecord.read(last_seen_static))
                  intfc.warn_if_ref_is_wrong_type(last_seen_static, form_type::statik, subrecord.signature());
               break;
            case 'DATA':
               {
                  size_t count = subrecord.size() / 0x1C;
                  if (!count)
                     break;
                  if (!last_seen_static) {
                     orphaned_instances_count += count;
                     break;
                  }
                  {
                     form_instance_list* list = nullptr;
                     for (auto& item : this->forms) {
                        if (item.base_form == last_seen_static) {
                           list = &item;
                           break;
                        }
                     }
                     if (!list) {
                        auto& item = this->forms.emplace_back();
                        item.base_form = last_seen_static;
                        list = &item;
                     }
                     for (size_t i = 0; i < count; ++i) {
                        auto& item = list->instances.emplace_back();
                        subrecord.unchecked_read(item.pos.x);
                        subrecord.unchecked_read(item.pos.y);
                        subrecord.unchecked_read(item.pos.z);
                        subrecord.unchecked_read(item.rot.x);
                        subrecord.unchecked_read(item.rot.y);
                        subrecord.unchecked_read(item.rot.z);
                        subrecord.unchecked_read(item.scale);
                     }
                  }
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
      if (orphaned_instances_count > 0) {
         specific_load_warnings::orphaned_instances notice(
            this->stub,
            orphaned_instances_count
         );
         intfc.log_load_warning(notice);
      }
   }
   /*static*/ void StaticCollection::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      form_id_t last_seen_static;
      std::vector<form_id_t> retained_statics;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;

            case 'ONAM':
               subrecord.read(last_seen_static);
               break;
            case 'DATA':
               if (subrecord.size() < 0x1C)
                  break;
               {
                  bool already = false;
                  for (uint32_t id : retained_statics) {
                     if (id == last_seen_static) {
                        already = true;
                        break;
                     }
                  }
                  if (already)
                     break;
               }
               retained_statics.push_back(last_seen_static);
               break;
         }
      }
      for (auto id : retained_statics)
         uib.add_outbound_reference(id);
   }
   void StaticCollection::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (StaticCollection*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy);

      for (auto& item : copy->forms) {
         item.base_form.set(*copy, nullptr);
      }
      copy->forms.clear();
      {
         size_t size = this->forms.size();
         auto& src = this->forms;
         auto& dst = copy->forms;
         dst.resize(size);
         for (size_t i = 0; i < size; ++i) {
            dst[i].base_form.set(*copy, src[i].base_form);
            dst[i].instances = src[i].instances;
         }
      }
   }
   void StaticCollection::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      for (auto& item : this->forms) {
         record.write_formID_subrecord('ONAM', item.base_form);
         
         auto& subrecord = record.open_next_subrecord('DATA');
         for (auto& instance : item.instances) {
            subrecord.write(instance.pos.x);
            subrecord.write(instance.pos.y);
            subrecord.write(instance.pos.z);
            subrecord.write(instance.rot.x);
            subrecord.write(instance.rot.y);
            subrecord.write(instance.rot.z);
            subrecord.write(instance.scale);
         }
         subrecord.close();
      }
   }
   void StaticCollection::_clear_impl() noexcept {
      this->bounds.clear();
      this->model.clear(*this);
      this->script_data.clear(*this);

      for (auto& item : this->forms) {
         item.base_form.set(*this, nullptr);
      }
      this->forms.clear();
   }
   void StaticCollection::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);

      bool any_removed = false;
      for (auto& item : this->forms) {
         item.base_form.clear_if(*this, other);
         if (!item.base_form)
            any_removed = true;
      }
      if (any_removed) {
         std::erase_if(
            this->forms,
            [](const auto& item) {
               return !item.base_form;
            }
         );
      }
   }
}