#pragma once
#include <atomic>
#include <mutex>
#include <QImage>
#include <QObject>
#include <QPixmap>
#include <QVariant>

namespace DirectX {
   struct ScratchImage;
   struct TexMetadata;
}
namespace dovahscript::core::subsystems {
   class resources;
}

namespace dovahscript {
   enum class resource_type {
      undefined = -1,
      //
      binary,
      dds,
      raster,
   };

   template<typename T, bool ui> requires (std::is_base_of_v<QObject, T>) class DovahscriptResourceBaseHandleImpl;

   class DovahscriptResource : public QObject {
      Q_OBJECT;

      friend class core::subsystems::resources;
      template<typename T, bool ui> requires (std::is_base_of_v<QObject, T>) friend class DovahscriptResourceBaseHandleImpl;
      
      protected:
         resource_type type = resource_type::undefined;
         std::atomic<int> refcount    = 0;
         std::atomic<int> ui_refcount = 0;
         bool desynchronized = false;
         struct {
            struct {
               DirectX::ScratchImage* data = nullptr;
               DirectX::TexMetadata*  info = nullptr;
            } dds;
            struct {
               QByteArray script;
               QByteArray client;
            } binary;
            struct {
               QImage script;
               QImage client; // also used for DDS
            } raster;
         } content;
         
      public:
         ~DovahscriptResource();

         std::atomic<bool> is_lua_referenced = false;

         static DovahscriptResource* make_dds(const void* buffer, size_t size);

         inline const resource_type resource_type() const noexcept { return this->type; }

         inline const QImage get_raster_script_side() const noexcept { return this->content.raster.script; }
         inline const QImage get_raster_widget_side() const noexcept { return this->content.raster.client; }
         
         void reserve_binary_script_side(size_t);
         void modify_binary_script_side(std::function<void(QByteArray&)> task); // Accessor to let Lua scripts modify binary data.
         void modify_raster_script_side(std::function<void(QImage&)> task); // Accessor to let Lua scripts modify image data.

         inline const QByteArray get_binary_script_side() const noexcept { return this->content.binary.script; }
         inline const QByteArray get_binary_widget_side() const noexcept { return this->content.binary.client; }

         inline bool is_dds() const noexcept { return this->type == resource_type::dds; }
         bool   is_cubemap()         const noexcept;
         size_t texture_array_size() const noexcept; // returns 1 for a non-array; 0 on failure. // NOTE: textures in an array can be mipmapped
         size_t mipmap_count()       const noexcept; // returns 0 for a non-mipmapped image
         QImage get_dds_layer(size_t array_index, size_t mipmap_index, uint8_t cubemap_face = 0) const noexcept; // returns a detached QImage
         //
         inline const DirectX::TexMetadata*  get_dds_metadata() const noexcept { return this->content.dds.info; }
         inline const DirectX::ScratchImage* get_dds_raw_data() const noexcept { return this->content.dds.data; }

      protected:
         void on_referenced(bool ui);
         void on_severed(bool ui);

         void resynchronize();
         void abandon_client_thread_content();

      signals:
         void resynchronized();
   };

   #pragma region Handle setup
   //
   // Unfortunately, Qt's meta-type system only supports smart pointers that are templated on their 
   // pointed-to type; I'm guessing they built it to make QPointer and friends work under the hood, 
   // and then exposed it for users. This means that we have to do some annoying indirection in 
   // order to get things working the way we want.
   //

   template<typename T, bool ui> requires (std::is_base_of_v<QObject, T>) class DovahscriptResourceBaseHandleImpl { // Qt needs it to be templated :(
      protected:
         using value_t = T;
         using self_t = DovahscriptResourceBaseHandleImpl<T, ui>;
         friend class DovahscriptResourceBaseHandleImpl<T, !ui>;
      protected:
         value_t* resource = nullptr;

         // It seems we can't reliably control the order in which widgets are deleted during VM teardown. 
         // Several sub-widgets are managed by Qt directly; Qt decides when and how to delete those; Qt 
         // decides when and how to delete their models... We can't guarantee a safe deletion order, so 
         // this connection is here to ensure we never need to.
         QMetaObject::Connection connection;

         void _inc() {
            if (resource) {
               resource->on_referenced(ui);
               connection = QObject::connect(resource, &QObject::destroyed, [this]() {
                  this->resource = nullptr;
               });
            }
         }
         void _dec() {
            if (connection)
               QObject::disconnect(connection);
            if (resource)
               resource->on_severed(ui);
         }

      public:
         DovahscriptResourceBaseHandleImpl() {}
         DovahscriptResourceBaseHandleImpl(value_t* v) : resource(v) {
            this->_inc();
         }
         DovahscriptResourceBaseHandleImpl(const DovahscriptResourceBaseHandleImpl& other) { *this = other; }
         DovahscriptResourceBaseHandleImpl(DovahscriptResourceBaseHandleImpl&& other) { *this = std::move(other); }
         DovahscriptResourceBaseHandleImpl(const DovahscriptResourceBaseHandleImpl<T, !ui>& other) {
            this->_dec();
            this->resource = other.resource;
            this->_inc();
         }
         DovahscriptResourceBaseHandleImpl(DovahscriptResourceBaseHandleImpl<T, !ui>&& other) {
            this->_dec();
            this->resource = other.resource;
            other.resource = nullptr;
            this->resource->on_referenced(ui);
            this->resource->on_severed(!ui);
         }
         ~DovahscriptResourceBaseHandleImpl() {
            this->_dec();
            this->resource = nullptr;
         }

         operator bool() { return this->resource != nullptr; };
         operator value_t*() const noexcept { return this->resource; };
         value_t* operator->() const noexcept { return this->resource; };

         DovahscriptResourceBaseHandleImpl& operator=(const DovahscriptResourceBaseHandleImpl& other) noexcept {
            this->_dec();
            this->resource = other.resource;
            this->_inc();
            return *this;
         }
         DovahscriptResourceBaseHandleImpl& operator=(DovahscriptResourceBaseHandleImpl&& other) noexcept {
            this->_dec();
            this->resource = other.resource;
            other.resource = nullptr;
            return *this;
         }
         DovahscriptResourceBaseHandleImpl& operator=(DovahscriptResourceBaseHandleImpl<T, !ui>&& other) noexcept {
            this->_dec();
            this->resource = other.resource;
            other.resource = nullptr;
            this->resource->on_referenced(ui);
            this->resource->on_severed(!ui);
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

   template<typename T> requires (std::is_base_of_v<QObject, T>) class DovahscriptResourceHandleImpl;
   template<typename T> requires (std::is_base_of_v<QObject, T>) class DovahscriptResourceUIHandleImpl;

   template<typename T> requires (std::is_base_of_v<QObject, T>) class DovahscriptResourceHandleImpl : public DovahscriptResourceBaseHandleImpl<T, false> {
      using DovahscriptResourceBaseHandleImpl<T, false>::DovahscriptResourceBaseHandleImpl;
   };
   template<typename T> requires (std::is_base_of_v<QObject, T>) class DovahscriptResourceUIHandleImpl : public DovahscriptResourceBaseHandleImpl<T, true> {
      using DovahscriptResourceBaseHandleImpl<T, true>::DovahscriptResourceBaseHandleImpl;
   };
   #pragma endregion

   using DovahscriptResourceUIHandle = DovahscriptResourceUIHandleImpl<DovahscriptResource>;
   using DovahscriptResourceHandle   = DovahscriptResourceHandleImpl<DovahscriptResource>;
}

// These macros don't work from within a namespace. Ignore IntelliSense errors on them, too; those may be false-positives.
Q_DECLARE_SMART_POINTER_METATYPE(dovahscript::DovahscriptResourceUIHandleImpl);
Q_DECLARE_METATYPE(dovahscript::DovahscriptResourceUIHandle);
Q_DECLARE_SMART_POINTER_METATYPE(dovahscript::DovahscriptResourceHandleImpl);
Q_DECLARE_METATYPE(dovahscript::DovahscriptResourceHandle);