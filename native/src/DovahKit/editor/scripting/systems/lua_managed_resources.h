#pragma once
#include <mutex>
#include "../../../helpers/singleton.h"
#include "editor_script_inner_core.h"

class DovahKitScriptVMResourceInterface;
class ObservableStandardItemModel;

namespace editor_script {
   template<typename T> requires std::is_base_of_v<QObject, T> class LuaManagedResourceHandleImpl;

   class LuaManagedResource : public QObject {
      Q_OBJECT;

      friend class DovahKitScriptVMResourceInterface;
      template<typename T> requires std::is_base_of_v<QObject, T> friend class LuaManagedResourceHandleImpl;
      
      using model_t = ObservableStandardItemModel;

      protected:
         int refcount = 0;
         QHash<model_t*, int> model_refcounts;
      public:
         bool is_lua_referenced = false;
         struct {
            struct {
               QImage  script;
               QPixmap client;
            } raster;
         } content;

         // Script thread should call this when changing the image content.
         void mark_dirty();

      protected:
         void on_referenced(model_t*);
         void on_severed(model_t*);

         void resynchronize();

      signals:
         void resynchronized();
   };

   template<typename T> requires std::is_base_of_v<QObject, T> class LuaManagedResourceHandleImpl { // Qt needs it to be templated :(
      protected:
         using model_t = ObservableStandardItemModel;
         using value_t = T;
      protected:
         value_t* resource = nullptr;
         model_t* model    = nullptr;

         void _inc() {
            if (resource)
               resource->on_referenced(model);
         }
         void _dec() {
            if (resource)
               resource->on_severed(model);
         }

      public:
         LuaManagedResourceHandleImpl() {}
         LuaManagedResourceHandleImpl(value_t* v, model_t* m) : resource(v), model(m) {
            this->_inc();
         }
         LuaManagedResourceHandleImpl(const LuaManagedResourceHandleImpl& other) { *this = other; }
         LuaManagedResourceHandleImpl(LuaManagedResourceHandleImpl&& other) { *this = other; }
         ~LuaManagedResourceHandleImpl() {
            this->_dec();
            this->resource = nullptr;
            this->model    = nullptr;
         }

         operator bool() { return this->resource != nullptr; };
         operator value_t*() const noexcept { return this->resource; };
         value_t* operator->() const noexcept { return this->resource; };

         LuaManagedResourceHandleImpl& operator=(const LuaManagedResourceHandleImpl& other) noexcept {
            this->_dec();
            this->resource = other.resource;
            this->model    = other.model;
            this->_inc();
            return *this;
         }
         LuaManagedResourceHandleImpl& operator=(LuaManagedResourceHandleImpl&& other) noexcept {
            this->_dec();
            this->resource = other.resource;
            this->model    = other.model;
            other.resource = nullptr;
            other.model    = nullptr;
            return *this;
         }
   };

   using LuaManagedResourceHandle = LuaManagedResourceHandleImpl<LuaManagedResource>;
}

//
// Manage resources such as rasters that are in use by scripts.
//
class DovahKitScriptVMResourceInterface : cobb::singleton {
   using resource_t      = editor_script::LuaManagedResource;
   using resource_list_t = QList<resource_t*>;
   using handle_t        = editor_script::LuaManagedResourceHandle;
   friend class DovahKitScriptVMCore;
   protected:
      struct locked_resource_list {
         std::mutex      lock;
         resource_list_t list;
      };

      struct {
         locked_resource_list desynched;
         locked_resource_list extant;
         locked_resource_list pending_deletion;
      } resources;

      DovahKitScriptVMResourceInterface() {
         qRegisterMetaType<handle_t>(); // ensure the metatype is registered at run-time
      }

      // Call when tearing down the VM, after all widgets are gone.
      void clear();

   public:
      static DovahKitScriptVMResourceInterface& get() {
         static DovahKitScriptVMResourceInterface instance;
         return instance;
      }

      void main_thread_handler(); // VM core should call this from the main thread; nothing else should touch it

      // Creates a resource and returns it. This must be called from inside of a script-to-client cross-thread task, and 
      // the script thread MUST receive the resource, store it in a Lua wrapper, and flag it as Lua-referenced.
      //
      // TODO: Ideally, the "push wrapper to Lua" function should set the Lua-referenced flag only when actually pushing 
      // the copied wrapper into Lua.
      //
      resource_t* create_resource(QImage source = QImage());

      void on_resource_unreferenced(resource_t&); // call when the resource becomes Lua-unreferenced or Qt-unreferenced

      void mark_dirty(resource_t&);
};

// These macros don't work from within a namespace. Ignore IntelliSense errors on them, too; those may be false-positives.
Q_DECLARE_SMART_POINTER_METATYPE(editor_script::LuaManagedResourceHandleImpl);
Q_DECLARE_METATYPE(editor_script::LuaManagedResourceHandle);