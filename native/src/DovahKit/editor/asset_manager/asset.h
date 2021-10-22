#pragma once
#include <QObject>

#include <QImage>
namespace DirectX {
   struct ScratchImage;
   struct TexMetadata;
}

class DovahKitAssetHandle;
class DovahKitAssetManager;

class DovahKitAsset : public QObject {
   Q_OBJECT;
   friend class DovahKitAssetHandle;
   friend class DovahKitAssetManager;
   public:
      enum Type {
         Undefined,
         DDS,
         NIF,
      };
      Q_ENUM(Type);

   protected:
      Type _type = Type::Undefined;
      struct {
         struct {
            uint32_t all           = 0;
            uint32_t other_assets  = 0;
            uint32_t render_window = 0;
         } refcounts;
         bool content_loaded      = false;
         bool content_failed      = false;
         bool dependencies_loaded = false;
      } _state;
      struct {
         struct {
            DirectX::ScratchImage* data = nullptr;
            DirectX::TexMetadata*  info = nullptr;
         } dds;
         QImage image;
      } _data;

      void on_handle_made();
      void on_handle_lost();

   public:
      DovahKitAsset(Type, QObject* parent = nullptr);
      ~DovahKitAsset();

      inline Type type() const noexcept { return this->_type; }

      inline bool areDependenciesLoaded() const noexcept { return this->_state.dependencies_loaded; }
      inline bool didContentLoadingFail() const noexcept { return this->_state.content_failed; }
      inline bool isContentLoaded() const noexcept { return this->_state.content_loaded; }
      inline bool isReady() const noexcept { return this->isContentLoaded() && this->areDependenciesLoaded(); }

      inline const DirectX::ScratchImage* ddsImage()    const noexcept { return this->_data.dds.data; }
      inline const DirectX::TexMetadata*  ddsMetadata() const noexcept { return this->_data.dds.info; }

      inline QImage image() const noexcept { return this->_data.image; }

   protected slots:
      void load(const QString& path);

   signals:
      void contentLoaded();        // The asset's own content has loaded.
      void contentLoadingFailed(); // The asset's own content has failed to load.
      void dependenciesLoaded();   // The asset depends on other assets, and they have all finished loading.
      void ready();                // The asset is ready for use: its own content and the content of any dependencies is all loaded.
};

class DovahKitAssetHandle {
   public:
      using value_type = DovahKitAsset;
      enum Flag {
         IsRenderWindow = 0x00000001, // This handle is being used by the Render Window's 3D view.
      };
      Q_DECLARE_FLAGS(Flags, Flag);

   protected:
      value_type* asset = nullptr;

      QMetaObject::Connection connection;

      void _inc() {
         if (asset) {
            asset->on_handle_made();
            connection = QObject::connect(asset, &QObject::destroyed, [this]() {
               this->asset = nullptr;
            });
         }
      }
      void _dec() {
         if (connection)
            QObject::disconnect(connection);
         if (asset)
            asset->on_handle_lost();
      }

   public:
      DovahKitAssetHandle() {}
      DovahKitAssetHandle(value_type* v);
      DovahKitAssetHandle(const DovahKitAssetHandle& other);
      DovahKitAssetHandle(DovahKitAssetHandle&& other);
      ~DovahKitAssetHandle();

      Flags flags = 0;

      operator bool() { return this->asset != nullptr; };
      operator value_type*() const noexcept { return this->asset; };
      value_type* operator->() const noexcept { return this->asset; };

      DovahKitAssetHandle& operator=(const DovahKitAssetHandle& other) noexcept;
      DovahKitAssetHandle& operator=(DovahKitAssetHandle&& other) noexcept;

      inline value_type* bare() const noexcept { return this->asset; }
};
Q_DECLARE_OPERATORS_FOR_FLAGS(DovahKitAssetHandle::Flags);