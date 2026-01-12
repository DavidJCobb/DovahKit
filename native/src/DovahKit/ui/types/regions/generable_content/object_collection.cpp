#include "./object_collection.h"
#include "dovah/data/bound_object_form_types.h"
#include "dovah/forms/Region.h"

namespace ui::types::regions::generable_content {
   void object_collection::clear() {
      this->objects.clear();
   }
   void object_collection::import_data(const dovah::loaded_forms::Region& src_form) {
      this->clear();

      for (auto& src_coll : src_form.generable_content) {
         auto* src_cast = src_coll.as<backend_collection_type>();
         if (!src_cast)
            continue;
         this->import_data(*src_cast);
      }
   }
   void object_collection::import_data(const backend_collection_type& src_coll) {
      std::vector<object*> flat_list;

      auto& src_list = src_coll.objects;
      flat_list.reserve(flat_list.size() + src_list.size());
      for (size_t i = 0; i < src_list.size(); ++i) {
         auto& src_item  = src_list[i];
         auto* base_form = src_item.form.get_form_stub();
         if (!base_form || !allows_form_type(base_form->form_type)) {
            flat_list.push_back(nullptr);
            continue;
         }
         auto  dst_item_ptr = std::make_unique<object>();
         auto& dst_item     = *dst_item_ptr;
         dst_item.data.base_form = base_form;
         dst_item.data.params    = src_item.params;
         if (src_item.parent_index < 0) {
            this->objects.push_back(std::move(dst_item_ptr));
         } else if (src_item.parent_index >= i) {
            flat_list.push_back(nullptr);
            continue;
         } else {
            auto* parent = flat_list[src_item.parent_index];
            if (parent) {
               parent->children.push_back(std::move(dst_item_ptr));
               dst_item.clamp_slope_to_ancestor_range(false);
            } else {
               flat_list.push_back(nullptr);
               continue;
            }
            dst_item.parent = parent;
         }
         flat_list.push_back(&dst_item);
      }
   }
   void object_collection::export_data(dovah::loaded_forms::Region& dst_form, backend_collection_type& dst_coll) const {
      dst_coll.clear(dst_form);

      for (auto& node_ptr : this->objects) {
         [&dst_coll, &dst_form](this auto&& recurse, object& subject, int parent_index) -> void {
            size_t subject_index = dst_coll.objects.size();
            auto&  dst_item      = dst_coll.objects.emplace_back();
            dst_item.form.set(dst_form, subject.data.base_form);
            dst_item.parent_index = parent_index;
            dst_item.params       = subject.data.params;
            for (auto& child_ptr : subject.children) {
               recurse(*child_ptr, subject_index);
            }
         }(*node_ptr, -1);
      }
   }

   /*static*/ bool object_collection::allows_form_type(dovah::form_type ft) {
      for (auto candidate : dovah::bound_object_form_types)
         if (candidate == ft)
            return true;
      return false;
   }
   object_collection& object_collection::operator=(const object_collection& src) {
      std::vector<std::unique_ptr<object>> my_prior_objects;
      std::swap(my_prior_objects, this->objects);
      try {
         auto dupe_subtree = [](this auto&& recurse, object& src) -> std::unique_ptr<object> {
            auto  dst_ptr = std::make_unique<object>();
            auto& dst     = *dst_ptr;
            dst.data = src.data;
            dst.children.reserve(src.children.size());
            for (auto& src_child_ptr : src.children) {
               auto dst_child_ptr = recurse(*src_child_ptr);
               dst_child_ptr->parent = &dst;
               dst.children.push_back(std::move(dst_child_ptr));
            }
            return dst_ptr;
         };
         this->objects.reserve(src.objects.size());
         for (auto& src_node_ptr : src.objects) {
            auto dst_node_ptr = dupe_subtree(*src_node_ptr);
            this->objects.push_back(std::move(dst_node_ptr));
         }
      } catch (...) {
         std::swap(my_prior_objects, this->objects); // restore what we had previously
         my_prior_objects.clear(); // explicitly destroy any incomplete duplicates
         throw;
      }
      return *this;
   }
}