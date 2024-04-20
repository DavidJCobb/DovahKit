#include "./game_setting_edit_request.h"
#include "../files/file_load_order.h"

namespace dovah {
   game_setting_edit_request::game_setting_edit_request(file_load_order& o, form_id_policy p) : owner(o), policy(p) {
   }
   game_setting_edit_request::game_setting_edit_request(game_setting_edit_request&& other) : owner(other.owner), policy(other.policy) {
      this->setting    = other.setting;
      this->desiredID  = other.desiredID;
      this->reservedID = other.reservedID;
   }
   game_setting_edit_request::~game_setting_edit_request() {
      this->owner.abandon_form_id_reservation(*this);
   }
   void game_setting_edit_request::acquire_form_id() {
      if (this->policy != form_id_policy::find_valid_id) {
         #if _DEBUG
            __debugbreak(); // Calling this doesn't make sense if you're not relying on the load order to pick a form ID for you.
         #endif
         return;
      }
      if (this->desiredID)
         return;
      //
      this->owner.set_reserved_form_id_for(*this);
   }
   void game_setting_edit_request::set_desired_form_id(bare_form_id_t id) {
      if (this->policy != form_id_policy::use_chosen_id) {
         #if _DEBUG
            __debugbreak(); // Calling this doesn't make sense if you're not supplying a form ID yourself.
         #endif
         return;
      }
      if (this->desiredID == id)
         return;
      //
      this->owner.set_reserved_form_id_for(*this, id);
   }
   void game_setting_edit_request::commit() {
      this->owner.commit_game_setting_change_request(*this);
   }
}