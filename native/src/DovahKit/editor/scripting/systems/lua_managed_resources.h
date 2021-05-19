#pragma once
#include <functional>
#include <mutex>
#include "../../../helpers/singleton.h"
#include "editor_script_inner_core.h"
#include <QStyledItemDelegate>

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
         struct {
            struct {
               QImage  script;
               QPixmap client;
            } raster;
         } content;
      public:
         bool is_lua_referenced = false;

         inline const QImage  get_raster_script_side() const noexcept { return this->content.raster.script; }
         inline const QPixmap get_raster_widget_side() const noexcept { return this->content.raster.client; }
         void modify_raster_script_side(std::function<void(QImage)> task); // Accessor to let Lua scripts modify image data. Refer to function on VM subsystem for further info.

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

         inline value_t* bare() const noexcept { return this->resource; }

         static value_t* extract_from_variant(const QVariant& data) noexcept {
            if (data.isValid() && data.canConvert<QObject*>()) {
               if (auto* object = data.value<QObject*>())
                  return qobject_cast<value_t*>(object);
            }
            return nullptr;
         }
   };

   using LuaManagedResourceHandle = LuaManagedResourceHandleImpl<LuaManagedResource>;
}

class DovahKitScriptItemDelegate : public QStyledItemDelegate {
   public:
      using QStyledItemDelegate::QStyledItemDelegate;

      virtual void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
};

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

      #if _DEBUG
      // You can tamper with these in a debugger to queue checks to run.
      struct {
         bool double_check_lua_references = false;
      } debug;
      #endif

      DovahKitScriptVMResourceInterface() {
         qRegisterMetaType<handle_t>(); // ensure the metatype is registered at run-time
      }

      // Call when tearing down the VM, after all widgets are gone. Calling before all widgets are gone may result in 
      // double-frees when those widgets' contained LuaManagedResourceHandles run destructors, as those try to act on 
      // resources that were already deleted.
      //
      // This function uses QObject::deleteLater on the resources, so you should probably use deleteLater on the UI 
      // widgets first.
      //
      void clear();

   public:
      static DovahKitScriptVMResourceInterface& get() {
         static DovahKitScriptVMResourceInterface instance;
         return instance;
      }

      void _run_queued_debug_functions();

      void main_thread_handler(); // VM core should call this from the main thread; nothing else should touch it

      // Creates a resource and returns it. This must be called from inside of a script-to-client cross-thread task, and 
      // the script thread MUST receive the resource and push it into Lua via a wrapper.
      resource_t* create_resource(QImage source = QImage());

      // Accessor to allow Lua APIs to modify a resource's raster content on the script thread; for convenience, you can 
      // also call this on the resource object itself. This function locks the desynchronized resource list for the full 
      // duration of whatever task you pass in, to avoid race conditions that could lead to the image being modified 
      // while the main thread is resynchronizing it (which would be especially bad when wholly overwriting the image, 
      // as is needed for a resize).
      void modify_raster_script_side(resource_t&, std::function<void(QImage&)> task);

      void on_resource_unreferenced(resource_t&); // call when the resource becomes Lua-unreferenced or Qt-unreferenced
};

// These macros don't work from within a namespace. Ignore IntelliSense errors on them, too; those may be false-positives.
Q_DECLARE_SMART_POINTER_METATYPE(editor_script::LuaManagedResourceHandleImpl);
Q_DECLARE_METATYPE(editor_script::LuaManagedResourceHandle);