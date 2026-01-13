#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "./form_creation_request.h"

namespace dovah {
   class file_load_order;
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
         std::optional<form_creation_request::grid_coordinates> cell_grid_coordinates; // grid coordinates to use when duplicating an exterior cell
         
         void set_target(form_stub* original);
         
         void set_parent_form(form_stub* parent);
         void set_parent_form(bare_form_id_t parentID);
         
         form_stub* commit();
         
         size_t get_total_form_count() const noexcept;
   };
}