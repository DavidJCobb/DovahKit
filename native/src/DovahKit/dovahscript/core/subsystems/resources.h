#pragma once
#include <mutex>
#include <QElapsedTimer>
#include "../../../helpers/singleton.h"
#include "../../../lua.h"
#include "resources/DovahscriptResource.h"

namespace dovahscript::core::subsystems {
   class resources : cobb::singleton {
      using resource_t      = DovahscriptResource;
      using resource_list_t = QList<resource_t*>;
      using handle_t        = DovahscriptResourceHandle;
      public:
         static resources& get() {
            static resources instance;
            return instance;
         }

      protected:
         struct locked_resource_list {
            std::mutex      lock;
            resource_list_t list;
         };

         QElapsedTimer timer;
         struct {
            locked_resource_list desynched;
            locked_resource_list extant;
            locked_resource_list pending_deletion;
         } stored_resources;

         resources() {
            qRegisterMetaType<handle_t>(); // ensure the metatype is registered at run-time
         }

      public:
         void on_script_setup();
         void main_thread_handler(); // VM core should call this from the main thread; nothing else should touch it
         void on_script_teardown();

         // Creates a resource and returns it. This must be called from inside of a script-to-client cross-thread task, and 
         // the script thread MUST receive the resource and push it into Lua via a wrapper.
         resource_t* create_resource(QImage source = QImage());

         // Creates a resource from a buffer. Can return nullptr if the desired buffer does not contain valid data for the 
         // desired resource type (e.g. trying to create a DDS resource from non-DDS data).
         resource_t* create_resource(const QByteArray&, resource_type type = resource_type::binary);

         // Accessor to allow Lua APIs to modify a resource's raster content on the script thread; for convenience, you can 
         // also call this on the resource object itself.
         void modify_raster_script_side(resource_t&, std::function<void(QImage&)> task);

         void modify_binary_script_side(resource_t&, std::function<void(QByteArray&)> task);

         void reserve_binary_script_side(resource_t&, size_t);

         void on_resource_task_referenced_changed(resource_t&, bool became_referenced); // call when the resource becomes (un)referenced within C++
         void on_resource_ui_referenced_changed(resource_t&, bool became_referenced); // call when the resource becomes (un)referenced within the UI
         void on_resource_unreferenced(resource_t&); // call when the resource becomes Lua-unreferenced
   };
}