#include "./custom.h"
#include <memory>
#include "../../_common_cpp.h"

#include "../custom_packages/procedure_node.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   /*virtual*/ void custom::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) /*override*/ {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() != header_subrecord)
         return;

      //
      // Load header.
      //
      uint32_t package_data_count = 0;
      subrecord.read(package_data_count);
      if (auto& form = this->template_package; subrecord.read(form)) {
         intfc.warn_if_ref_is_wrong_type(form, form_type::package, subrecord.signature());
      }
      subrecord.read(this->revision);
      //
      // NOTE: If the revision number can't be stored in a uint16_t without truncation, 
      // then the Creation Kit will warn about it.
      //

      this->data.values.load(record, intfc, package_data_count);

      if (!this->template_package) {
         #pragma region Procedure tree
         {
            std::vector<std::unique_ptr<procedure_node>> nodes;
            while (record.get_current_subrecord().signature() == 'ANAM') {
               auto  node_ptr = std::make_unique<procedure_node>();
               auto* node     = node_ptr.get();
               nodes.push_back(std::move(node_ptr));

               node->load(record, intfc);
            }
            if (!nodes.empty()) {
               std::vector<procedure_node*> bare_ptrs;
               bare_ptrs.resize(nodes.size());
               for (size_t i = 0; i < nodes.size(); ++i)
                  bare_ptrs[i] = nodes[i].get();

               auto _traverse = [&bare_ptrs, &nodes](this auto&& recurse, procedure_node* parent, size_t parent_index) -> size_t {
                  size_t count = parent->child_count();

                  size_t index = parent_index + 1;
                  for (size_t i = 0; i < count; ++i) {
                     auto* child     = bare_ptrs[index + i];
                     auto& child_ptr = nodes[index + i];
                     
                     static_assert(
                        false,
                        "TODO: This sucks. We should have a variant with just two members (`procedure` and `branch`), and "
                        "have an enum on `branch` that indicates the branch type. (Currently the variant as a whole is "
                        "synched with the `procedure_node_type` enum.)"
                     );
                     switch (parent->data.index()) {
                        case 1: std::get<1>(parent->data).children[i] = std::move(child_ptr); break;
                        case 2: std::get<2>(parent->data).children[i] = std::move(child_ptr); break;
                        case 3: std::get<3>(parent->data).children[i] = std::move(child_ptr); break;
                        case 4: std::get<4>(parent->data).children[i] = std::move(child_ptr); break;
                     }
                     if (child->data.index() != 0) {
                        recurse(child, index + i);
                        index += child->child_count();
                     }
                  }
                  return index;
               };

               size_t i = 0;
               while (i < bare_ptrs.size()) {
                  i = _traverse(bare_ptrs[i], i);
               }
               if (i < bare_ptrs.size()) {
                  static_assert(false, "TODO: Warn: Multiple top-level nodes; the others will be discarded");
               }
               this->procedure_tree = std::move(nodes[0]);
            }
         }
         #pragma endregion
         //
         this->data.declarations.load(record, intfc);
      }
   }
   /*static*/ void custom::generate_header_use_info(tes_record_reader& record, std::vector<form_id_t>& out) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() != header_subrecord)
         return;

      form_id_t template_package;

      //
      // Load header.
      //
      uint32_t package_data_count = 0;
      subrecord.read(package_data_count);
      subrecord.read(template_package);
      record.next_subrecord();

      static_assert(false, "TODO: Mirror the load process above.");
      custom_packages::package_data_value_map::generate_use_info(record, package_data_count, out);
      if (template_package) {
         static_assert(
            false,
            "TODO: Procedure tree. We don't need to build a tree structure, but we do need a "
            "recursive function which, after reading PRCB, will read the next N children; and "
            "then we'd ignore any non-children past the first."
         );
         custom_packages::package_data_declaration_map::generate_use_info(record, out);
      }
   }
   /*virtual*/ void custom::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      if (this->data.values.entries.size() > custom_packages::package_data_value_map::max_serializable_count) {
         static_assert(false, "TODO: throw save error");
      }

      auto& subrecord = record.open_next_subrecord(header_subrecord);
      subrecord.write((uint32_t)this->data.values.entries.size());
      subrecord.write(this->template_package);
      subrecord.write(this->revision);
      subrecord.close();

      this->data.values.save(record, intfc);
      if (this->template_package) {
         static_assert(false, "TODO: Clear procedure tree, since we won't be saving it.");
         this->data.declarations = {};
      } else {
         #pragma region Procedure tree
            if (this->procedure_tree) {
               this->procedure_tree->save(record, intfc);
               static_assert(false, "TODO: Recursively serialize children");
            }
         #pragma endregion
         this->data.declarations.save(record, intfc);
      }
   }
   /*virtual*/ base* custom::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  copy_ptr = std::make_unique<custom>();
      auto* copy     = copy_ptr.get();

      copy->template_package.set(owner_of_clone, this->template_package);
      copy->revision = this->revision;

      copy->data.values.clone_from(this->data.values, owner_of_clone);
      static_assert(false, "TODO: `procedure_tree`");
      copy->data.declarations = this->data.declarations;

      return copy_ptr.release();
   }
   /*virtual*/ void custom::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      this->template_package.clear_if(my_owner, other);
      this->data.values.sever_outbound_references_to(other, my_owner);
      static_assert(false, "TODO: `procedure_tree`");
   }
   /*virtual*/ void custom::clear(loaded_forms::Form& my_owner) /*override*/ {
      this->template_package.set(my_owner, nullptr);

      this->data.values.clear(my_owner);
      static_assert(false, "TODO: `procedure_tree`");
      this->data.declarations = {};
   }
}