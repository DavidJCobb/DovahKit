#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../core.h"
#include "../notice_code_t.h"

namespace dovah {
   class file_load_order;
   class form_creation_request;
   class form_stub;
}

namespace dovah {
   class form_duplication_request {
      protected:
         file_load_order& owner;
         form_creation_request* main_request = nullptr;
         std::vector<form_creation_request*> child_requests;
         
         form_stub* parent = nullptr; // if the original form has a parent and you want the clone to have a different parent, use this. (nullptr) defaults to same parent.
         
         form_duplication_request(form_duplication_request&&);
         form_duplication_request(const form_duplication_request&) = delete;
         form_duplication_request& operator=(const form_duplication_request&) = delete;
      public:
         form_duplication_request(file_load_order& o);
         ~form_duplication_request();
         
         std::string editorID;
         struct {
            int32_t x = 0;
            int32_t y = 0;
            bool    present = false;
         } cell_grid_coordinates; // grid coordinates to use when duplicating an exterior cell
         
         void set_target(form_stub* original);
         
         void set_parent_form(form_stub* parent);
         void set_parent_form(bare_form_id_t parentID);
         
         form_stub* commit();
         
         notice_code_t get_main_form_error_code() const noexcept;
         std::vector<notice_code_t> get_child_form_error_codes() const noexcept; // returns only non-none errors
         std::vector<notice_code_t> get_error_codes() const noexcept; // returns all error codes, including nones. main first, then children
         bool has_error() const noexcept;
         bool is_valid() const noexcept;
         size_t get_total_form_count() const noexcept;
   };
}