#include "ActorValueInfo.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/actor_value_info/unexpected_subrecord_in_perk_tree_node.h"
#include "../notices/form_load_warnings/by_form_type/actor_value_info/unterminated_perk_tree_node.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::actor_value_info;
   }
}

namespace dovah::loaded_forms {
   void ActorValueInfo::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'DESC':
               subrecord.read(this->description);
               break;

            case 'ANAM':
               subrecord.read(this->abbreviation);
               break;
            case 'CNAM':
               subrecord.read(this->skill_info.category);
               break;
            case 'AVSK':
               subrecord.read(this->skill_info.skill_use_mult);
               subrecord.read(this->skill_info.skill_use_offset);
               subrecord.read(this->skill_info.skill_improve_mult);
               subrecord.read(this->skill_info.skill_improve_offset);
               break;
            case 'ICON':
               subrecord.read(this->icon);
               break;
            case 'SNAM':
               subrecord.read(this->skill_info.category);
               break;
            case 'PNAM':
               {
                  auto& node = this->perk_tree_nodes.emplace_back();
                  if (auto& form = node.perk; subrecord.read(form)) {
                     intfc.warn_if_ref_is_wrong_type(form, form_type::perk, subrecord.signature());
                  }
                  bool unterminated = true;
                  while (auto& subrecord = record.next_subrecord()) {
                     bool stop = false;
                     switch (subrecord.signature()) {
                        case 'FNAM':
                           subrecord.read(node.flags);
                           break;
                        case 'XNAM':
                           subrecord.read(node.x);
                           break;
                        case 'YNAM':
                           subrecord.read(node.y);
                           break;
                        case 'HNAM':
                           subrecord.read(node.position.h);
                           break;
                        case 'VNAM':
                           subrecord.read(node.position.v);
                           break;
                        case 'SNAM':
                           subrecord.read(node.skill);
                           break;
                        case 'CNAM':
                           subrecord.read(node.connections.emplace_back());
                           break;
                        case 'INAM':
                           subrecord.read(node.id);
                           stop = true;
                           break;

                        default:
                           specific_load_warnings::unexpected_subrecord_in_perk_tree_node notice(
                              this->stub,
                              node.perk.get_form_stub(),
                              subrecord.signature()
                           );
                           intfc.log_load_warning(notice);
                           break;
                     }
                     if (stop) {
                        unterminated = false;
                        break;
                     }
                  }
                  if (unterminated) {
                     specific_load_warnings::unterminated_perk_tree_node notice(
                        this->stub,
                        node.perk.get_form_stub()
                     );
                     intfc.log_load_warning(notice);
                  }
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void ActorValueInfo::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;

            case 'PNAM':
               {
                  form_id_t perk;
                  if (subrecord.read(perk))
                     uib.add_outbound_reference(perk);

                  form_id_t skill;
                  while (auto& subrecord = record.next_subrecord()) {
                     bool stop = false;
                     switch (subrecord.signature()) {
                        case 'SNAM':
                           subrecord.read(skill);
                           break;
                        case 'INAM':
                           stop = true;
                           break;
                     }
                     if (stop)
                        break;
                  }
                  if (skill)
                     uib.add_outbound_reference(skill);
               }
               break;
         }
      }
   }
   void ActorValueInfo::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (ActorValueInfo*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->name = this->name;
      copy->description = this->description;

      copy->abbreviation = this->abbreviation;
      copy->icon = this->icon;
      copy->skill_info = this->skill_info;

      {
         auto& src_list = this->perk_tree_nodes;
         auto& dst_list = copy->perk_tree_nodes;
         for (auto& node : dst_list) {
            node.perk.set(*copy, nullptr);
            node.skill.set(*copy, nullptr);
         }
         dst_list.clear();

         size_t size = src_list.size();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            auto& src_node = src_list[i];
            auto& dst_node = dst_list[i];
            dst_node.connections = src_node.connections;
            dst_node.flags       = src_node.flags;
            dst_node.id          = src_node.id;
            dst_node.perk.set(*copy, src_node.perk);
            dst_node.position    = src_node.position;
            dst_node.skill.set(*copy, src_node.skill);
            dst_node.x = src_node.x;
            dst_node.y = src_node.y;
         }
      }
   }
   void ActorValueInfo::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('FULL');
         subrecord.write(this->name);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DESC');
         subrecord.write(this->description);
         subrecord.close();
      }
      if (!this->abbreviation.empty()) {
         record.write_string_subrecord('ANAM', this->abbreviation);
      }
      {
         auto& subrecord = record.open_next_subrecord('CNAM');
         subrecord.write(this->skill_info.category);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('AVSK');
         subrecord.write(this->skill_info.skill_use_mult);
         subrecord.write(this->skill_info.skill_use_offset);
         subrecord.write(this->skill_info.skill_improve_mult);
         subrecord.write(this->skill_info.skill_improve_offset);
         subrecord.close();
      }
      for (auto& node : this->perk_tree_nodes) {
         record.write_formID_subrecord('PNAM', node.perk);
         {
            auto& subrecord = record.open_next_subrecord('FNAM');
            subrecord.write(node.flags);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('XNAM');
            subrecord.write(node.x);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('YNAM');
            subrecord.write(node.y);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('HNAM');
            subrecord.write(node.position.h);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('VNAM');
            subrecord.write(node.position.v);
            subrecord.close();
         }
         record.write_formID_subrecord('SNAM', node.skill);
         for (auto id : node.connections) {
            auto& subrecord = record.open_next_subrecord('CNAM');
            subrecord.write(id);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('INAM');
            subrecord.write(node.id);
            subrecord.close();
         }
      }
   }
   void ActorValueInfo::_clear_impl() noexcept {
      this->script_data.clear(*this);
      
      this->name.reset();
      this->description.reset();

      this->abbreviation = {};
      this->icon = {};
      this->skill_info = {};

      for (auto& node : this->perk_tree_nodes) {
         node.perk.set(*this, nullptr);
         node.skill.set(*this, nullptr);
      }
      this->perk_tree_nodes.clear();
   }
   void ActorValueInfo::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);

      for (auto& node : this->perk_tree_nodes) {
         node.perk.clear_if(*this, other);
         node.skill.clear_if(*this, other);
      }
   }
}