#include "./game_setting_renumber_request.h"
#include "../files/file_load_order.h"

namespace dovah {
   game_setting_renumber_request::game_setting_renumber_request(file_load_order& o) : owner(o) {
   }
   game_setting_renumber_request::game_setting_renumber_request(game_setting_renumber_request&& other) : owner(other.owner) {
      this->setting    = other.setting;
      this->desiredID  = other.desiredID;
      this->reservedID = other.reservedID;
      this->code       = other.code;
      this->done       = other.done;
   }
   game_setting_renumber_request::~game_setting_renumber_request() {
      this->owner.abandon_form_id_reservation(*this);
   }
   void game_setting_renumber_request::set_desired_form_id(bare_form_id_t id) {
      if (this->desiredID == id)
         return;
      this->owner.set_reserved_form_id_for(*this, id);
   }
   void game_setting_renumber_request::commit() {
      this->owner.commit_game_setting_renumber_request(*this);
   }
}