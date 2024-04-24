#pragma once
#include <string>
#include "../files/tes_file_writing/config.h"
#include "../files/tes_file_writing/file_writer.h"
#include "../utils/file_prefix.h"

namespace dovah {
   class file_load_order;
}

namespace dovah::load_order_processes {
   class file_save {
      protected:
         using writer_type = tes_file_writing::file_writer;

      public:
         file_save(file_load_order& subject) : active_load_order(subject) {}

      public:
         file_load_order& active_load_order;
         std::string      desired_filename;
         tes_file_writing::write_config write_config;
         struct {
            std::string filename;
            bool        saved_to_temporary_file = false;
         } results;
      protected:
         file_prefix _old_active_file_prefix;
         bool _save_as_light_plugin = false;
         bool _was_originally_light = false;


      public:
         // May throw:
         //  - dovah::exceptions::file_save_failed
         //  - dovah::exceptions::game_change_failed
         void execute();

      protected:
         void _post_save_form_id_remap();
         void _post_save_form_stub_file_info_update(writer_type& writer);
         void _post_save_unsaved_form_delete(writer_type& writer);

         std::vector<form_stub*> _find_stubs_to_discard_post_save(writer_type& writer);
   };
}
