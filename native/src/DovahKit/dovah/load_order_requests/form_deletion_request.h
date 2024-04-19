#pragma once
#include <set>
#include <vector>
#include "../utils/file_prefix.h"
#include "../notice_code_t.h"

namespace dovah {
   class file_load_order;
   class form_stub;
}

namespace dovah {
   class form_deletion_request {
      friend file_load_order;
      protected:
         file_load_order& owner;
         form_stub&       target;
         notice_code_t    error = default_notice_code;
         file_prefix      active_file_prefix; // cached for faster checks
         bool             done = false;
         //
         std::set<form_stub*> seen_stubs;
         std::set<form_stub*> forms_needing_delete;
         std::set<form_stub*> forms_needing_flag;
         
         form_deletion_request(file_load_order& o, form_stub& t);
         form_deletion_request(form_deletion_request&&);
         form_deletion_request(const form_deletion_request&) = delete;
         form_deletion_request& operator=(const form_deletion_request&) = delete;
         
         bool _form_should_be_flagged(form_stub&);
         void _gather_others(form_stub* start = nullptr);
         void _prep_for_delete(form_stub&, bool flag); // use for deletion and for flagging as deleted
         
      public:
         bool force_delete_overrides = false; // if (true), then we will straight-up delete ALL forms. if (false), then forms outside the active file are overridden and FLAGGED AS deleted.
         
         std::vector<form_stub*> get_forms_pending_delete(bool include_flagged = true) const noexcept;
         std::vector<form_stub*> get_forms_pending_flagging() const noexcept;
         constexpr notice_code_t get_error_code() const noexcept { return this->error; }
         
         void commit(); // cannot produce or signal errors. if no errors already occurred, then once you call this, you're locked in.
   };
}