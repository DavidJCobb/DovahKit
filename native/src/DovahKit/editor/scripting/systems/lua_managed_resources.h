#pragma once
#include <functional>
#include <mutex>
#include <QElapsedTimer>
#include <QImage>
#include <QStyledItemDelegate>
#include "../../../helpers/singleton.h"
#include "editor_script_inner_core.h"

namespace DirectX {
   struct ScratchImage;
   struct TexMetadata;
}
class DovahKitScriptVMResourceInterface;
class ObservableStandardItemModel;

namespace editor_script {
   template<typename T> requires (std::is_base_of_v<QObject, T>) class LuaManagedResourceHandleImpl;

   enum class lua_managed_resource_type {
      undefined = -1,
      //
      binary,
      dds,
      raster,
   };

   // The QImage format that other script APIs expect.
   extern constexpr QImage::Format desired_qt_pixel_format = QImage::Format_ARGB32_Premultiplied;

   class LuaManagedResource : public QObject {
      Q_OBJECT;

      friend class DovahKitScriptVMResourceInterface;
      template<typename T> requires (std::is_base_of_v<QObject, T>) friend class LuaManagedResourceHandleImpl;

      protected:
         lua_managed_resource_type type = lua_managed_resource_type::undefined;
         std::atomic<int> refcount = 0;
         struct {
            QByteArray binary; // for unknown-type resources
            struct {
               DirectX::ScratchImage* data = nullptr;
               DirectX::TexMetadata*  info = nullptr;
            } dds;
            struct {
               QImage  script;
               QPixmap client;
            } raster;
         } content;
         
      public:
         ~LuaManagedResource();

         std::atomic<bool> is_lua_referenced = false;

         static LuaManagedResource* make_dds(const void* buffer, size_t size);

         inline const lua_managed_resource_type resource_type() const noexcept { return this->type; }

         inline const QImage  get_raster_script_side() const noexcept { return this->content.raster.script; }
         inline const QPixmap get_raster_widget_side() const noexcept { return this->content.raster.client; }
         void modify_raster_script_side(std::function<void(QImage&)> task); // Accessor to let Lua scripts modify image data. Refer to function on VM subsystem for further info.

         inline const QByteArray get_binary_script_side() const noexcept { return this->content.binary; }

         inline bool is_dds() const noexcept { return this->type == lua_managed_resource_type::dds; }
         bool   is_cubemap()         const noexcept;
         size_t texture_array_size() const noexcept; // returns 1 for a non-array; 0 on failure. // NOTE: textures in an array can be mipmapped
         size_t mipmap_count()       const noexcept; // returns 0 for a non-mipmapped image
         QImage get_dds_layer(size_t array_index, size_t mipmap_index, uint8_t cubemap_face = 0) const noexcept;

      protected:
         void on_referenced();
         void on_severed();

         void resynchronize();

      signals:
         void resynchronized();
   };

   template<typename T> requires (std::is_base_of_v<QObject, T>) class LuaManagedResourceHandleImpl { // Qt needs it to be templated :(
      protected:
         using value_t = T;
      protected:
         value_t* resource = nullptr;

         // It seems we can't reliably control the order in which widgets are deleted during VM teardown. 
         // Several sub-widgets are managed by Qt directly; Qt decides when and how to delete those; Qt 
         // decides when and how to delete their models... We can't guarantee a safe deletion order, so 
         // this connection is here to ensure we never need to.
         QMetaObject::Connection connection;

         void _inc() {
            if (resource) {
               resource->on_referenced();
               connection = QObject::connect(resource, &QObject::destroyed, [this]() {
                  this->resource = nullptr;
               });
            }
         }
         void _dec() {
            if (connection)
               QObject::disconnect(connection);
            if (resource)
               resource->on_severed();
         }

      public:
         LuaManagedResourceHandleImpl() {}
         LuaManagedResourceHandleImpl(value_t* v) : resource(v) {
            this->_inc();
         }
         LuaManagedResourceHandleImpl(const LuaManagedResourceHandleImpl& other) { *this = other; }
         LuaManagedResourceHandleImpl(LuaManagedResourceHandleImpl&& other) { *this = std::move(other); }
         ~LuaManagedResourceHandleImpl() {
            this->_dec();
            this->resource = nullptr;
         }

         operator bool() { return this->resource != nullptr; };
         operator value_t*() const noexcept { return this->resource; };
         value_t* operator->() const noexcept { return this->resource; };

         LuaManagedResourceHandleImpl& operator=(const LuaManagedResourceHandleImpl& other) noexcept {
            this->_dec();
            this->resource = other.resource;
            this->_inc();
            return *this;
         }
         LuaManagedResourceHandleImpl& operator=(LuaManagedResourceHandleImpl&& other) noexcept {
            this->_dec();
            this->resource = other.resource;
            other.resource = nullptr;
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

      QElapsedTimer timer;
      struct {
         locked_resource_list desynched;
         locked_resource_list extant;
         locked_resource_list pending_deletion;
      } resources;

      DovahKitScriptVMResourceInterface() {
         qRegisterMetaType<handle_t>(); // ensure the metatype is registered at run-time
      }

      void clear();

   public:
      static DovahKitScriptVMResourceInterface& get() {
         static DovahKitScriptVMResourceInterface instance;
         return instance;
      }

      void main_thread_handler(); // VM core should call this from the main thread; nothing else should touch it

      // Creates a resource and returns it. This must be called from inside of a script-to-client cross-thread task, and 
      // the script thread MUST receive the resource and push it into Lua via a wrapper.
      resource_t* create_resource(QImage source = QImage());

      // Creates a resource from a buffer. Can return nullptr if the desired buffer does not contain valid data for the 
      // desired resource type (e.g. trying to create a DDS resource from non-DDS data).
      resource_t* create_resource(const QByteArray&, editor_script::lua_managed_resource_type type = editor_script::lua_managed_resource_type::binary);

      // Accessor to allow Lua APIs to modify a resource's raster content on the script thread; for convenience, you can 
      // also call this on the resource object itself. This function locks the desynchronized resource list for the full 
      // duration of whatever task you pass in, to avoid race conditions that could lead to the image being modified 
      // while the main thread is resynchronizing it (which would be especially bad when wholly overwriting the image, 
      // as is needed for a resize).
      void modify_raster_script_side(resource_t&, std::function<void(QImage&)> task);

      void on_resource_ui_referenced_changed(resource_t&, bool became_referenced); // call when the resource becomes Qt-(un)referenced
      void on_resource_unreferenced(resource_t&); // call when the resource becomes Lua-unreferenced
};

// These macros don't work from within a namespace. Ignore IntelliSense errors on them, too; those may be false-positives.
Q_DECLARE_SMART_POINTER_METATYPE(editor_script::LuaManagedResourceHandleImpl);
Q_DECLARE_METATYPE(editor_script::LuaManagedResourceHandle);