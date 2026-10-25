#pragma once
#include <vector>
#include "dovah/_passkeys/add_form_stub_source_file.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_collection.h"
#include "dovah/form_type.h"
#include "dovah/types/bare_form_id.h"

namespace dovah::impl::_form_stub_collection {
   //
   // This is in its own file so that `form_stub::_add_source_file` can be friends with it, but 
   // not with the entire `form_stub_collection`.
   //
   inline void create_none_stubs(form_stub_collection& self, tes::file& none_stubs_file) {
      std::vector<bare_form_id> used_form_ids;
      std::vector<form_stub*>   users;
      for (auto& pair : self.forms) {
         auto* stub = pair.second;
         if (!stub)
            continue;
         bool found_any = false;
         for (auto& pair : stub->outbound) {
            if (pair.second.other) // if there's a stub pointer, then this isn't a dangling form-to-form reference
               continue;
            used_form_ids.push_back(pair.first);
            found_any = true;
         }
         if (found_any)
            users.push_back(stub);
      }
      if (used_form_ids.empty())
         return;
      for (auto id : used_form_ids) {
         auto* stub = new form_stub;
         stub->form_id   = id;
         stub->form_type = form_type::none;
         stub->_add_source_file({}, none_stubs_file, 0, 0);
         //
         self.forms_by_type[form_type::none][id] = stub;
         self.forms[id] = stub;
      }
      //
      // Simply creating the none-stubs isn't enough, though: the use info in the 
      // referencing forms needs to be updated to point to the new none-stubs.
      //
      for (auto* user : users) {
         for (auto& pair : user->outbound) {
            if (pair.second.other)
               continue;
            pair.second.other = self.get_form_by_id(pair.first, form_type::none);
         }
      }
   }
}