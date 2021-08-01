#pragma once
#include <QImage>
#include <QObject>
#include <QPixmap>

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

   template<typename T> requires (std::is_base_of_v<QObject, T>) class DovahscriptResourceHandleImpl;

   class DovahscriptResource : public QObject {
      Q_OBJECT;

      friend class core::subsystems::resources;
      template<typename T> requires (std::is_base_of_v<QObject, T>) friend class DovahscriptResourceHandleImpl;
      
      protected:
         resource_type type = resource_type::undefined;
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
         ~DovahscriptResource();

         std::atomic<bool> is_lua_referenced = false;

         static DovahscriptResource* make_dds(const void* buffer, size_t size);

         inline const resource_type resource_type() const noexcept { return this->type; }

         inline const QImage  get_raster_script_side() const noexcept { return this->content.raster.script; }
         inline const QPixmap get_raster_widget_side() const noexcept { return this->content.raster.client; }
         void modify_raster_script_side(std::function<void(QImage&)> task); // Accessor to let Lua scripts modify image data.

         inline const QByteArray get_binary_script_side() const noexcept { return this->content.binary; }

         inline bool is_dds() const noexcept { return this->type == resource_type::dds; }
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

   template<typename T> requires (std::is_base_of_v<QObject, T>) class DovahscriptResourceHandleImpl { // Qt needs it to be templated :(
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
         DovahscriptResourceHandleImpl() {}
         DovahscriptResourceHandleImpl(value_t* v) : resource(v) {
            this->_inc();
         }
         DovahscriptResourceHandleImpl(const DovahscriptResourceHandleImpl& other) { *this = other; }
         DovahscriptResourceHandleImpl(DovahscriptResourceHandleImpl&& other) { *this = std::move(other); }
         ~DovahscriptResourceHandleImpl() {
            this->_dec();
            this->resource = nullptr;
         }

         operator bool() { return this->resource != nullptr; };
         operator value_t*() const noexcept { return this->resource; };
         value_t* operator->() const noexcept { return this->resource; };

         DovahscriptResourceHandleImpl& operator=(const DovahscriptResourceHandleImpl& other) noexcept {
            this->_dec();
            this->resource = other.resource;
            this->_inc();
            return *this;
         }
         DovahscriptResourceHandleImpl& operator=(DovahscriptResourceHandleImpl&& other) noexcept {
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

   using DovahscriptResourceHandle = DovahscriptResourceHandleImpl<DovahscriptResource>;
}

// These macros don't work from within a namespace. Ignore IntelliSense errors on them, too; those may be false-positives.
Q_DECLARE_SMART_POINTER_METATYPE(dovahscript::DovahscriptResourceHandleImpl);
Q_DECLARE_METATYPE(dovahscript::DovahscriptResourceHandle);