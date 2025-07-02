#include "./procedure_node.h"
#include "../../_common_cpp.h"
#include <array>
#include <utility>

namespace {
   constexpr const auto typename_mapping = []() {
      using type = dovah::packages::procedure_node_type;
      using pair = std::pair<std::string_view, type>;
      return std::array{
         pair{ "Procedure", type::procedure },
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

      procedure_node_type type = (procedure_node_type)-1;
      for (const auto& item : typename_mapping) {
         if (item.first == serialized_typename) {
            type = item.second;
            break;
         }
      }
      if (type == (procedure_node_type)-1) {
         return;
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

      switch (type) {
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) case procedure_node_type::name: this->data.emplace<(size_t)procedure_node_type::name>().load(record, intfc); break;
         CASE(procedure);
         CASE(random);
         CASE(sequence);
         CASE(simultaneous);
         CASE(stacked);
         #pragma pop_macro("CASE")

         default:
            specific_load_warnings::procedure_node_unrecognized_typename notice(
               intfc.target_stub,
               serialized_typename
            );
            intfc.log_load_warning(notice);
            break;
      }
   }
   /*static*/ void procedure_node::generate_use_info(tes_record_reader& record, std::vector<form_id_t>& out);
   void procedure_node::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      {
         std::string_view name = "";
         const auto       type = this->get_type();
         for (auto& item : typename_mapping) {
            if (item.second == type) {
               name = item.first;
               break;
            }
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
      std::visit(
         [&record, &intfc](auto& v) {
            v.save(record, intfc);
         },
         this->data
      );
   }
   procedure_node* procedure_node::clone(loaded_forms::Form& owner_of_clone) const noexcept;
   void procedure_node::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept;
   void procedure_node::clear(loaded_forms::Form& my_owner);
}