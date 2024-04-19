#include "./form_renumber_request.h"
#include "../files/file_load_order.h"

namespace dovah {
   form_renumber_request::form_renumber_request(file_load_order& o, form_stub& t) : owner(o), target(t) {
   }
   form_renumber_request::form_renumber_request(form_renumber_request&& other) : owner(other.owner), target(other.target) {
      this->desiredID = other.desiredID;
   }
   form_renumber_request::~form_renumber_request() {
      this->owner.abandon_form_id_reservation(*this);
   }
   void form_renumber_request::commit() {
      this->owner.commit_form_renumber_request(*this);
   }
}