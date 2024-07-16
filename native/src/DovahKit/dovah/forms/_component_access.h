#pragma once

namespace dovah::loaded_forms {
   class Form;
   namespace components {
      namespace papyrus {
         class attachment_data;
      }
      class  leveled_list;
      class  model;
      struct object_bounds;
      using  papyrus_attachment_data = papyrus::attachment_data;
   }
}

namespace dovah::loaded_forms::component_access {
   extern components::leveled_list* get_leveled_list(Form*);
   extern components::model* get_model(Form*);
   extern components::object_bounds* get_object_bounds(Form*);
   extern components::papyrus_attachment_data* get_papyrus_data(Form*);
}