#include "./procedure_node.h"
#include "../../_common_cpp.h"
#include <array>
#include <utility>

#include "../../../notices/form_load_warnings/by_form_type/package/procedure_node_unrecognized_typename.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

namespace {
   constexpr const std::string_view leaf_node_typename = "Procedure";

   constexpr const auto branch_typename_mapping = []() {
      using type = dovah::packages::procedure_tree_branch_type;
      using pair = std::pair<std::string_view, type>;
      return std::array{
         pair{ "Random", type::random },
         pair{ "Sequence", type::sequence },
         pair{ "Simultaneous", type::simultaneous },
         pair{ "Stacked", type::stacked },
      };
   }();
}

namespace dovah::loaded_forms::structs::custom_packages {
   void procedure_node::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      if (record.get_current_subrecord().signature() != subrecord_typename) {
         static_assert("TODO: Warn");
         return;
      }
      std::string serialized_typename;
      record.get_current_subrecord().read(serialized_typename);
      record.next_subrecord();

      std::optional<packages::procedure_tree_branch_type> branch_type;
      bool is_leaf = false;
      if (serialized_typename == leaf_node_typename) {
         is_leaf = true;
      } else {
         for (const auto& item : branch_typename_mapping) {
            if (item.first == serialized_typename) {
               branch_type = item.second;
               break;
            }
         }
         if (!branch_type.has_value()) {
            specific_load_warnings::procedure_node_unrecognized_typename notice(
               intfc.target_stub,
               serialized_typename
            );
            intfc.log_load_warning(notice);
         }
      }

      if (auto& subrecord = record.get_current_subrecord(); subrecord.signature() == 'CITC') {
         uint32_t count = 0;
         subrecord.read(count);
         record.next_subrecord();
         this->conditions.reserve(count);
      }
      while (record.get_current_subrecord().signature() == 'CTDA') {
         this->conditions.read_next(record, intfc);
      }

      if (is_leaf) {
         this->data.emplace<procedure_node_data::procedure>().load(record, intfc);
      } else if (branch_type.has_value()) {
         auto& data = this->data.emplace<procedure_node_data::branch>();
         data.branch_type = branch_type.value();
         data.load(record, intfc);
      } else {
         auto& data = this->data.emplace<procedure_node_data::unknown>();
         data.serialized_typename = serialized_typename;
      }
   }
   /*static*/ void procedure_node::generate_use_info(tes_record_reader& record, std::vector<form_id_t>& out);
   void procedure_node::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      {
         std::string_view name = "";
         if (std::holds_alternative<procedure_node_data::procedure>(this->data)) {
            name = leaf_node_typename;
         } else if (auto* casted = std::get_if<procedure_node_data::branch>(&this->data)) {
            auto type = casted->branch_type;
            for (auto& item : branch_typename_mapping) {
               if (item.second == type) {
                  name = item.first;
                  break;
               }
            }
         } else if (auto* casted = std::get_if<procedure_node_data::unknown>(&this->data)) {
            name = casted->serialized_typename;
         }
         record.write_string_subrecord(subrecord_typename, name.data());
      }
      {
         auto& subrecord = record.open_next_subrecord('CITC');
         subrecord.write((uint32_t)this->conditions.size());
         subrecord.close();
         for (auto& cnd : this->conditions)
            cnd.save(record, intfc);
      }
      if (auto* casted = std::get_if<procedure_node_data::procedure>(&this->data)) {
         casted->save(record, intfc);
      } else if (auto* casted = std::get_if<procedure_node_data::branch>(&this->data)) {
         casted->save(record, intfc);
      }
   }
   procedure_node* procedure_node::clone(loaded_forms::Form& owner_of_clone) const noexcept {
      auto  copy_ptr = std::make_unique<procedure_node>();
      auto* copy     = copy_ptr.get();

      copy->conditions.append_all_of(owner_of_clone, this->conditions);
      if (auto* casted = std::get_if<procedure_node_data::procedure>(&this->data)) {
         copy->data.emplace<procedure_node_data::procedure>() = *casted;
      } else if (auto* casted = std::get_if<procedure_node_data::branch>(&this->data)) {
         auto& src_data = *casted;
         auto& dst_data = copy->data.emplace<procedure_node_data::branch>();
         dst_data.branch_type = src_data.branch_type;
         dst_data.flags       = src_data.flags;
         static_assert(false, "TODO: Recursively clone child nodes");
      } else if (auto* casted = std::get_if<procedure_node_data::unknown>(&this->data)) {
         copy->data.emplace<procedure_node_data::unknown>() = *casted;
      }

      return copy_ptr.release();
   }
   void procedure_node::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept;
   void procedure_node::clear(loaded_forms::Form& my_owner);
}