#include "./procedure_tree.h"
#include "../../_common_cpp.h"
#include <array>
#include <utility>
#include "./procedure_nodes/branch.h"

#include "../../../notices/form_load_warnings/by_form_type/package/procedure_tree_has_orphaned_nodes.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

namespace dovah::loaded_forms::structs::custom_packages {
   void procedure_tree::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      struct loaded_node {
         std::unique_ptr<procedure_node> ptr;
         size_t child_count = 0;
         bool branch_trailing_content = false;
      };
      std::vector<loaded_node> nodes;

      while (record.get_current_subrecord().signature() == procedure_node::subrecord_typename) {
         auto  node_ptr = std::make_unique<procedure_node>();
         auto* node     = node_ptr.get();

         auto& entry = nodes.emplace_back();
         entry.ptr         = std::move(node_ptr);
         entry.child_count = node->load(record, intfc);

         //
         // HACK: Simultaneous nodes can have PFOR, and it comes after the subrecords for 
         //       their child/descendant nodes. Functionally, an "extra" PFOR after a 
         //       procedure will apply to the last seen Simultaneous node that hasn't 
         //       already taken a PFOR. Ditto for PFO2, but a Simultaneous node can only 
         //       load one (i.e. it'll claim the first in a sequence of PFOR+PFO2 or 
         //       PFO2+PFOR).
         // 
         // Example: [PACK:0005912A]DefaultMasterPackageMultiLinkDayTemplate
         // 
         //       Node #6 ends with a PFOR, followed by another PFOR for its ancestor 
         //       node #1.
         //
         while (
            record.get_current_subrecord().signature() == package_flag_overrides::subrecord_legacy ||
            record.get_current_subrecord().signature() == package_flag_overrides::subrecord_modern
         ) {
            const bool is_legacy = record.get_current_subrecord().signature() == package_flag_overrides::subrecord_legacy;

            bool found = false;
            for (auto rit = nodes.rbegin(); rit != nodes.rend(); ++rit) {
               auto& info = *rit;
               assert(!!info.ptr);
               if (info.branch_trailing_content) {
                  continue;
               }
               if (std::holds_alternative<procedure_node_data::branch>(info.ptr->data)) {
                  auto& b = std::get<procedure_node_data::branch>(info.ptr->data);
                  if (b.can_have_flag_overrides()) {
                     auto& fo = b.flag_overrides.emplace();
                     fo.load(record.get_current_subrecord(), intfc);
                     record.next_subrecord();
                     info.branch_trailing_content = true;
                     found = true;
                     break;
                  }
               }
            }
            if (!found) {
               //
               // TODO: warn
               //
            }
         }
      }

      if (nodes.empty())
         return;
      
      // Returns index to continue from.
      auto _acquire = [&nodes](this auto&& recurse, procedure_node& parent, size_t parent_index, size_t child_count) -> size_t {
         if (child_count == 0) {
            return parent_index + 1;
         }
         assert(std::holds_alternative<procedure_node_data::branch>(parent.data));
         auto& branch_data = std::get<procedure_node_data::branch>(parent.data);

         size_t child_index = parent_index + 1;
         for (size_t i = 0; i < child_count; ++i) {
            auto&  child_info  = nodes[child_index];
            auto&  child_ptr   = child_info.ptr;
            auto*  child       = child_ptr.get();

            branch_data.children.push_back(std::move(child_ptr));

            if (auto* casted = std::get_if<procedure_node_data::branch>(&child->data)) {
               child_index = recurse(*child, child_index, child_info.child_count);
            } else {
               ++child_index;
            }
         }
         return child_index;
      };
      
      this->root = std::move(nodes[0].ptr);
      size_t next = _acquire(
         *this->root,
         0,
         nodes[0].child_count
      );
      while (next < nodes.size()) {
         auto& orphan_info = nodes[next];
         auto* orphan      = orphan_info.ptr.get();
         this->orphans.push_back(std::move(orphan_info.ptr));
         next = _acquire(
            *orphan,
            next,
            orphan_info.child_count
         );
      }

      if (this->orphans.size()) {
         specific_load_warnings::procedure_tree_has_orphaned_nodes notice(
            intfc.target_stub,
            this->orphans.size()
         );
         intfc.log_load_warning(notice);
      }
   }
   /*static*/ void procedure_tree::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      while (record.get_current_subrecord().signature() == procedure_node::subrecord_typename) {
         procedure_node::generate_use_info(record, uib);
      }
   }
   void procedure_tree::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      /*
      
         So for some reason, Bethesda allows Simultaneous branch nodes to have package 
         flag overrides (hereafter: PFOs); and for some reason, they decided to have 
         the game and CK serialize a branch's PFOs *after* the subrecords for all of 
         the branch's children and descendants. This is a bizarre and hideous decision 
         that has forced me to write all sorts of disgusting hacks into this loader.

         If Bethesda *hadn't* done that, then saving the tree would be as simple as:

            this->for_each_node([&record, &intfc](procedure_node& node) {
               node.save(record, intfc);
            });

         Unfortunately, Bethesda's bad and weird and dumb decision has forced a much 
         more elaborate approach. The main thing: if a Simultaneous branch node has 
         PFOs, its last child is *capable* of having PFOs (by virtue of being either 
         a procedure node or another Simultaneous branch node), and its last child 
         *does not actually have* PFOs, then we have to serialize no-op PFOs onto its 
         last child. If we don't, then the PFOs we'd serialize onto the parent node 
         may be misread and loaded as PFOs belonging to the child.

         Note that Bethesda themselves *do not do this*; the Creation Kit will in fact 
         serialize PFOs incorrectly for a Simultaneous node that has them but also has 
         a last child that is capable of having them but doesn't.

      */
      this->for_each_node([](procedure_node& node) {
         if (!std::holds_alternative<procedure_node_data::branch>(node.data))
            return;
         auto& parent = std::get<procedure_node_data::branch>(node.data);
         if (!parent.can_have_flag_overrides() || !parent.flag_overrides.has_value())
            return;
         if (parent.children.empty())
            return;

         assert(!!parent.children.back());
         auto& child = *parent.children.back();
         if (std::holds_alternative<procedure_node_data::branch>(child.data)) {
            auto& b = std::get<procedure_node_data::branch>(child.data);
            if (b.can_have_flag_overrides() && !b.flag_overrides.has_value())
               b.flag_overrides.emplace();
            //
            // And then we'll recurse into this child and deal with the grandchildren.
            //
         } else if (std::holds_alternative<procedure_node_data::procedure>(child.data)) {
            auto& p = std::get<procedure_node_data::procedure>(child.data);
            if (!p.flag_overrides.has_value())
               p.flag_overrides.emplace();
         }
      });
      //
      // Now that we've dealt with that, we can get to saving the damn things.
      //
      this->for_each_node_with_trailer(
         [&record, &intfc](procedure_node& node) {
            bool force_flag_overrides = false;
            if (std::holds_alternative<procedure_node_data::branch>(node.data)) {
               auto& b = std::get<procedure_node_data::branch>(node.data);
               if (b.can_have_flag_overrides() && b.flag_overrides.has_value()) {
                  force_flag_overrides = true;
               }
            }
            node.save(record, intfc);
         },
         [&record, &intfc](procedure_node& node) {
            //
            // Save trailing content if present for Simultaneous nodes.
            // 
            if (std::holds_alternative<procedure_node_data::branch>(node.data)) {
               auto& b = std::get<procedure_node_data::branch>(node.data);
               if (b.can_have_flag_overrides()) {
                  if (b.flag_overrides.has_value()) {
                     auto& subrecord = record.open_next_subrecord(package_flag_overrides::subrecord_modern);
                     b.flag_overrides.value().save(subrecord, intfc);
                     subrecord.close();
                  }
               } else {
                  // Flag overrides illegally present; strip.
                  b.flag_overrides.reset();
               }
            }
         }
      );
   }
   void procedure_tree::clone_from(const procedure_tree& src, Form& my_owner) noexcept {
      this->clear(my_owner);
      if (src.root) {
         auto clone_ptr = src.root->clone(my_owner);
         this->root = std::move(clone_ptr);
      }
      for (auto& node_ptr : src.orphans) {
         auto clone_ptr = node_ptr->clone(my_owner);
         this->orphans.push_back(std::move(clone_ptr));
      }
   }
   void procedure_tree::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept {
      if (this->root)
         this->root->sever_outbound_references_to(other, my_owner);
      for (auto& node_ptr : this->orphans)
         node_ptr->sever_outbound_references_to(other, my_owner);
   }
   void procedure_tree::clear(loaded_forms::Form& my_owner) {
      if (this->root) {
         this->root->clear(my_owner);
         this->root.reset();
      }

      for (auto& node_ptr : this->orphans)
         node_ptr->clear(my_owner);
      this->orphans.clear();
   }
}