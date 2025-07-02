#include "./custom.h"
#include <bitset>
#include <memory>
#include "../../_common_cpp.h"

#include "../custom_packages/package_data.h"
#include "../custom_packages/procedure_node.h"

#include "../../../notices/form_load_warnings/by_form_type/package/package_data_metadata_belongs_to_none.h"
#include "../../../notices/form_load_warnings/by_form_type/package/package_data_wants_none_as_unique_id.h"
#include "../../../notices/form_load_warnings/by_form_type/package/too_many_package_data.h"
#include "../../../notices/form_load_warnings/by_form_type/package/unexpected_subrecord_in_unique_id_list.h"
#include "../../../notices/form_load_warnings/by_form_type/package/unique_id_has_multiple_metadata.h"
#include "../../../notices/form_load_warnings/by_form_type/package/unique_id_used_by_multiple_package_data.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

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

      #pragma region BGSPackageDataList
         if (package_data_count > 255) {
            specific_load_warnings::too_many_package_data notice(
               intfc.target_stub,
               package_data_count
            );
            intfc.log_load_warning(notice);
         }
         //
         // Spawn [our equivalent to] BGSPackageData objects and load their values.
         //
         for (uint32_t i = 0; i < package_data_count; ++i) {
            auto& subrecord = record.next_subrecord();

            custom_packages::package_data::load_context context;
            context.which = this->data.size();

            auto* packdata = custom_packages::package_data::load_content(record, intfc, context);
            this->data.push_back(packdata);
         }
         //
         // Load the mapping of BGSPackageData indices to unique IDs.
         //
         std::bitset<255> ids_already_assigned;
         for (uint32_t i = 0; i < package_data_count; ++i) {
            auto& subrecord = record.next_subrecord();
            auto* packdata  = this->data[i];
            switch (subrecord.signature()) {
               case 'UNAM':
                  {
                     uint8_t unique_id = package_data::no_unique_id;
                     if (subrecord.read(unique_id)) {
                        if (packdata) {
                           packdata->unique_id = unique_id;
                        }
                        if (unique_id == package_data::no_unique_id) {
                           specific_load_warnings::package_data_wants_none_as_unique_id notice(
                              intfc.target_stub,
                              i,
                              packdata ? packdata->get_type() : dovah::packages::package_data_type::invalid
                           );
                           intfc.log_load_warning(notice);
                        }
                     }
                     if (unique_id < ids_already_assigned.size()) {
                        if (ids_already_assigned.test(unique_id)) {
                           specific_load_warnings::unique_id_used_by_multiple_package_data notice(
                              intfc.target_stub,
                              unique_id
                           );
                           intfc.log_load_warning(notice);
                        }
                        ids_already_assigned.set(unique_id);
                     }
                  }
                  break;
               default:
                  {
                     specific_load_warnings::unexpected_subrecord_in_unique_id_list notice(
                        intfc.target_stub,
                        i,
                        subrecord.signature()
                     );
                     intfc.log_load_warning(notice);
                  }
                  this->next_unique_id = i + 1;
                  if (packdata) {
                     packdata->unique_id = i;
                  }
                  if (i == package_data::no_unique_id) {
                     specific_load_warnings::package_data_wants_none_as_unique_id notice(
                        intfc.target_stub,
                        i,
                        packdata ? packdata->get_type() : dovah::packages::package_data_type::invalid
                     );
                     intfc.log_load_warning(notice);
                  }
                  break;
            }
         }

         record.next_subrecord();

         //
         // Load next unique ID.
         //
         {
            auto& subrecord = record.get_current_subrecord();
            if (subrecord.signature() == 'XNAM') {
               subrecord.read(this->next_unique_id);
               record.next_subrecord();
            }
         }
      #pragma endregion

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
         #pragma region BGSPackageDataNameMap
            if (record.get_current_subrecord().signature() == 'UNAM') {
               std::bitset<255> used_ids;
               size_t which = 0;
               do {
                  uint8_t unique_id;
                  record.get_current_subrecord().read(unique_id);
                  record.next_subrecord();

                  if (unique_id == package_data::no_unique_id) {
                     specific_load_warnings::package_data_metadata_belongs_to_none notice(
                        intfc.target_stub,
                        which
                     );
                     intfc.log_load_warning(notice);
                  } else {
                     if (used_ids.test(unique_id)) {
                        specific_load_warnings::unique_id_has_multiple_metadata notice(
                           intfc.target_stub,
                           unique_id
                        );
                        intfc.log_load_warning(notice);
                     }
                     used_ids.set(unique_id);
                  }

                  package_data* current = nullptr;
                  if (unique_id != package_data::no_unique_id) {
                     for (auto* packdata : this->data) {
                        if (!packdata)
                           continue;
                        if (packdata->unique_id == unique_id) {
                           current = packdata;
                           break;
                        }
                     }
                  }

                  {  // Handle BNAM if present.
                     auto& subrecord = record.get_current_subrecord();
                     if (subrecord.signature() == 'BNAM') {
                        if (current) {
                           subrecord.read(current->name);
                        }
                        record.next_subrecord();
                     }
                  }
                  {  // Handle PNAM if present.
                     auto& subrecord = record.get_current_subrecord();
                     if (subrecord.signature() == 'PNAM') {
                        if (current) {
                           uint32_t dword;
                           subrecord.read(dword);
                           current->is_public = dword == 1;
                        }
                        record.next_subrecord();
                     }
                  }
                  //
                  // Done with this packdata.
                  //
               } while (++which, record.get_current_subrecord().signature() == 'UNAM');
            }
         #pragma endregion
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
      static_assert(false, "TODO: Header.");
      static_assert(false, "TODO: BGSPackageDataList.");
      if (template_package) {
         static_assert(
            false,
            "TODO: Procedure tree. We don't need to build a tree structure, but we do need a "
            "recursive function which, after reading PRCB, will read the next N children; and "
            "then we'd ignore any non-children past the first."
         );
         static_assert(false, "TODO: BGSPackageDataNameMap.");
      }
   }
   /*virtual*/ void custom::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      if (this->data.size() > max_serializable_package_data) {
         static_assert(false, "TODO: throw save error");
      }

      auto& subrecord = record.open_next_subrecord(header_subrecord);
      subrecord.write((uint32_t)this->data.size());
      subrecord.write(this->template_package);
      subrecord.write(this->revision);
      subrecord.close();

      #pragma region BGSPackageDataList
         static_assert(false, "TODO");
      #pragma endregion

      if (this->template_package) {
         static_assert(false, "TODO: Clear procedure tree, since we won't be saving it.");
         static_assert(false, "TODO: Clear package data name info, since we won't be saving it.");
      } else {
         #pragma region Procedure tree
            if (this->procedure_tree) {
               this->procedure_tree->save(record, intfc);
               static_assert(false, "TODO: Recursively serialize children");
            }
         #pragma endregion
         #pragma region BGSPackageDataNameMap
            static_assert(false, "TODO");
         #pragma endregion
      }
   }
   /*virtual*/ base* custom::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  copy_ptr = std::make_unique<custom>();
      auto* copy     = copy_ptr.get();

      copy->template_package.set(owner_of_clone, this->template_package);
      copy->revision = this->revision;

      for (auto* packdata : this->data) {
         if (!packdata) {
            copy->data.push_back(nullptr);
            break;
         }
         copy->data.push_back(packdata->clone(owner_of_clone));
      }
      copy->next_unique_id = this->next_unique_id;

      static_assert(false, "TODO: `procedure_tree`");

      return copy_ptr.release();
   }
   /*virtual*/ void custom::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      this->template_package.clear_if(my_owner, other);

      for (auto* packdata : this->data) {
         if (!packdata)
            continue;
         packdata->sever_outbound_references_to(other, my_owner);
      }

      static_assert(false, "TODO: `procedure_tree`");
   }
   /*virtual*/ void custom::clear(loaded_forms::Form& my_owner) /*override*/ {
      this->template_package.set(my_owner, nullptr);

      for (auto* packdata : this->data) {
         if (!packdata)
            continue;
         packdata->clear(my_owner);
      }
      this->data.clear();

      static_assert(false, "TODO: `procedure_tree`");
   }
}