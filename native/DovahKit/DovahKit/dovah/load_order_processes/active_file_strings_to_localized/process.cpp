#include "./process.h"
#include <cassert>
#include <unordered_map>
#include "dovah/active_load_order.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_collection.h"
#include "dovah/game_setting_collection.h"
#include "dovah/loaded_game_setting.h"
#include "dovah/localization/active_string_file_bundle.h"
#include "dovah/localization/string_collection.h"
#include "dovah/tes/file.h"
#include "dovah/types/bare_localized_string_id.h"
#include "dovah/utils/form_stubs/can_load_form_data.h"
#include "dovah/utils/localization/data_by_string_category.h"
#include "./convert_strings_in_form.h"
#include "./process_pimpl.h"

namespace dovah::load_order_processes::active_file_strings_to_localized {
   process::process(const process_params& p) : _params(p) {
      this->_pimpl = new process_pimpl({}, p);
   }
   process::~process() {
      if (auto* p = this->_pimpl) {
         this->_pimpl = nullptr;
         delete p;
      }
   }

   void process::execute() {
      auto& alo = this->_params.active_load_order;
      auto& fsc = alo.get_form_stub_collection();
      auto& gsc = alo.get_game_setting_collection();
      auto& lsc = alo.get_localized_string_collection();

      auto* active_file = alo.get_active_file();
      assert(!!active_file);
      assert(!active_file->header().flags.has_all_of<tes::file_header_flag::localized_strings>() && "Cannot convert a file that is already localized!");
      #if _DEBUG
         fsc.for_each_active_file_form([](form_stub& stub) {
            if (stub.form_type == form_type::setting)
               return;
            assert(utils::form_stubs::can_load_form_data(stub) && "We must be able to load all forms in the active file!");
         });
      #endif

      auto* active_bundle = lsc.bundle_for_active_file(); // TODO: passkeyed non-const accessor
      assert(!!active_bundle);
      assert(active_bundle->empty());

      fsc.for_each_active_file_form([this](form_stub& stub) {
         auto loaded_ptr = stub.load();
         assert(!!loaded_ptr);
         convert_strings_in_form(*this->_pimpl, stub, *loaded_ptr);
      });
      gsc.for_each_active_file_game_setting([this](loaded_game_setting& gmst) {
         this->_pimpl->convert_localized_string(gmst);
      });

      active_file->header().flags.set_each_of<tes::file_header_flag::localized_strings>();
   }
}