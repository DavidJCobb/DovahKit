#pragma once
#include <set>
#include <vector>
#include "../utils/file_prefix.h"

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
         file_prefix      active_file_prefix; // cached for faster checks
         //
         std::set<form_stub*> seen_stubs;
         std::set<form_stub*> forms_needing_delete;
         std::set<form_stub*> forms_needing_flag;
         
         form_deletion_request(file_load_order& o, form_stub& t);
         form_deletion_request(form_deletion_request&&);
         form_deletion_request(const form_deletion_request&) = delete;
         form_deletion_request& operator=(const form_deletion_request&) = delete;
         
         bool _form_should_be_flagged(form_stub&) noexcept;
         void _gather_others(form_stub* start = nullptr);
         void _prep_for_delete(form_stub&, bool flag); // use for deletion and for flagging as deleted
         
      public:
         bool delete_dialogue_children = true;  // if `true`, then deleting a quest deletes branches, topics, etc., even though they aren't literal child forms; and thus also to branches taking their topics with them
         bool force_delete_overrides   = false; // if (true), then we will straight-up delete ALL forms. if (false), then forms outside the active file are overridden and FLAGGED AS deleted.
         
         std::vector<form_stub*> get_forms_pending_delete(bool include_flagged = true) const noexcept;
         std::vector<form_stub*> get_forms_pending_flagging() const noexcept;
         
         void commit() noexcept;
   };
}