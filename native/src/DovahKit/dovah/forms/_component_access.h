#pragma once

namespace dovah::loaded_forms {
   class Form;
   namespace components {
      namespace papyrus {
         class script_data;
      }
      class model;
      using papyrus_attachment_data = papyrus::script_data;
   }
}

namespace dovah::loaded_forms::component_access {
   extern components::model* get_model(Form*);
   extern components::papyrus_attachment_data* get_papyrus_data(Form*);
}