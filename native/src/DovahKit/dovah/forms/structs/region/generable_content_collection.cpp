#include "./generable_content_collection.h"

namespace dovah::loaded_forms::structs::region {
   void generable_content_collection::clear(dovah::loaded_forms::Form& containing_form) {
      if (auto* casted = std::get_if<generable_content::raw_object_collection>(&this->data)) {
         casted->clear(containing_form);
      } else if (auto* casted = std::get_if<generable_content::weather_collection>(&this->data)) {
         casted->clear(containing_form);
      } else if (auto* casted = std::get_if<generable_content::grass_collection>(&this->data)) {
         casted->clear(containing_form);
      } else if (auto* casted = std::get_if<generable_content::audio>(&this->data)) {
         casted->clear(containing_form);
      }
      this->data.emplace<0>();
      this->override = false;
   }
   void generable_content_collection::clone_from(dovah::loaded_forms::Form& my_containing_form, const generable_content_collection& src_coll) {
      this->clear(my_containing_form);

      this->override = src_coll.override;
      this->priority = src_coll.priority;

      switch (src_coll.data.index()) {
         case 0:
            this->data.emplace<0>();
            return;
         case 1:
            this->data.emplace<1>();
            return;
      }
      if (auto* casted_src = std::get_if<generable_content::audio>(&src_coll.data)) {
         auto& casted_dst = this->data.emplace<generable_content::audio>();
         casted_dst.clone_from(my_containing_form, *casted_src);
      } else if (auto* casted_src = std::get_if<generable_content::grass_collection>(&src_coll.data)) {
         auto& casted_dst = this->data.emplace<generable_content::grass_collection>();
         casted_dst.clone_from(my_containing_form, *casted_src);
      } else if (auto* casted_src = std::get_if<generable_content::raw_object_collection>(&src_coll.data)) {
         auto& casted_dst = this->data.emplace<generable_content::raw_object_collection>();
         casted_dst.clone_from(my_containing_form, *casted_src);
      } else if (auto* casted_src = std::get_if<generable_content::weather_collection>(&src_coll.data)) {
         auto& casted_dst = this->data.emplace<generable_content::weather_collection>();
         casted_dst.clone_from(my_containing_form, *casted_src);
      } else if (auto* casted_src = std::get_if<generable_content::landscape>(&src_coll.data)) {
         auto& casted_dst = this->data.emplace<generable_content::landscape>();
         casted_dst = *casted_src;
      } else if (auto* casted_src = std::get_if<generable_content::map>(&src_coll.data)) {
         auto& casted_dst = this->data.emplace<generable_content::map>();
         casted_dst = *casted_src;
      }
   }
   void generable_content_collection::sever_references_to(dovah::loaded_forms::Form& containing_form, dovah::form_stub& stub) {
      if (auto* casted = std::get_if<generable_content::raw_object_collection>(&this->data)) {
         casted->sever_references_to(containing_form, stub);
      } else if (auto* casted = std::get_if<generable_content::weather_collection>(&this->data)) {
         casted->sever_references_to(containing_form, stub);
      } else if (auto* casted = std::get_if<generable_content::grass_collection>(&this->data)) {
         casted->sever_references_to(containing_form, stub);
      } else if (auto* casted = std::get_if<generable_content::audio>(&this->data)) {
         casted->sever_references_to(containing_form, stub);
      }
   }
}