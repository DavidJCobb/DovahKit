#pragma once
#include "../core.h"

namespace dovah {
   class form_stub;
   namespace tes_file_reading {
      class record;
   }
   namespace tes_file_writing {
      class record;
   }

   namespace loaded_forms {
      class Form {
         public:
            const form_type_t formType;
            Form(form_type_t ft) : formType(ft) {};
            //
            form_stub* stub = nullptr;
            //
            const char* get_editor_id() const noexcept;
            //
            uint32_t flags = 0;
            void load(tes_file_reading::record& record);

            //
            // This function returns a success bool.
            //
            // When you override it, you should call super so that you write the form's 
            // editor ID. However, you should NOT check the return value. The default 
            // function provided here on this class always returns false, so that form 
            // types without explicit save code can be handled properly. (In the future 
            // I may just make this a pure function and then require subclasses to check 
            // the result of calling super.)
            //
            virtual bool save(tes_file_writing::record& record);
      };
   }
}