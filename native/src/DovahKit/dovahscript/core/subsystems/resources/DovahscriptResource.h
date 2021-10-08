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

   // Forward-declarations
   class DovahscriptResource;
   class DovahscriptResourceHandle;
   template<typename T> class task_reference;
   namespace impl::task_reference {
      extern void inc(DovahscriptResource*);
      extern void dec(DovahscriptResource*);
   }

   class DovahscriptResource : public QObject {
      Q_OBJECT;

      friend class core::subsystems::resources;
      friend void  impl::task_reference::inc(DovahscriptResource*);
      friend void  impl::task_reference::dec(DovahscriptResource*);
      friend class DovahscriptResourceHandle;
      
      protected:
         resource_type type = resource_type::undefined;
         struct {
            std::atomic<int> task = 0;
            std::atomic<int> ui   = 0;
         } refcounts;
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

         void _on_task_referenced();
         void _on_task_unreferenced();
         
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
         void on_handle_made();
         void on_handle_lost();

         void resynchronize();
         void abandon_client_thread_content();

      signals:
         void resynchronized();
   };

   class DovahscriptResourceHandle {
      public:
         using value_type = DovahscriptResource;
      protected:
         DovahscriptResource* resource = nullptr;

         // It seems we can't reliably control the order in which widgets are deleted during VM teardown. 
         // Several sub-widgets are managed by Qt directly; Qt decides when and how to delete those; Qt 
         // decides when and how to delete their models... We can't guarantee a safe deletion order, so 
         // this connection is here to ensure we never need to.
         QMetaObject::Connection connection;

         void _inc() {
            if (resource) {
               resource->on_handle_made();
               connection = QObject::connect(resource, &QObject::destroyed, [this]() {
                  this->resource = nullptr;
               });
            }
         }
         void _dec() {
            if (connection)
               QObject::disconnect(connection);
            if (resource)
               resource->on_handle_lost();
         }

      public:
         DovahscriptResourceHandle() {}
         DovahscriptResourceHandle(DovahscriptResource* v);
         DovahscriptResourceHandle(const DovahscriptResourceHandle& other);
         DovahscriptResourceHandle(DovahscriptResourceHandle&& other);
         DovahscriptResourceHandle(const task_reference<DovahscriptResource>&);
         ~DovahscriptResourceHandle();

         operator bool() { return this->resource != nullptr; };
         operator DovahscriptResource*() const noexcept { return this->resource; };
         DovahscriptResource* operator->() const noexcept { return this->resource; };

         DovahscriptResourceHandle& operator=(const DovahscriptResourceHandle& other) noexcept;
         DovahscriptResourceHandle& operator=(DovahscriptResourceHandle&& other) noexcept;

         inline DovahscriptResource* bare() const noexcept { return this->resource; }

         static DovahscriptResource* extract_from_variant(const QVariant& data) noexcept;
   };
}

// These macros don't work from within a namespace. Ignore IntelliSense errors on them, too; those may be false-positives.
Q_DECLARE_METATYPE(dovahscript::DovahscriptResourceHandle);

// The Q_DECLARE_SMART_POINTER_METATYPE macro is only needed if we're declaring a smart pointer 
// meant to be templated on a QObject type. It exists so that a QVariant holding that smart 
// pointer can be directly converted to the pointed-to type, and so that QVariant::canConvert 
// can be queried for the pointed-to type directly.