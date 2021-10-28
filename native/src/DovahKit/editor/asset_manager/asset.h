#pragma once
#include <cstdint>
#include <mutex>
#include <QObject>
#include <QPointer>

#include <QImage>
namespace DirectX {
   struct ScratchImage;
   struct TexMetadata;
}

class DovahKitAssetReceptor;
class DovahKitAssetManager;

class DovahKitAsset : public QObject {
   Q_OBJECT;
   friend class DovahKitAssetReceptor;
   friend class DovahKitAssetManager;
   public:
      enum Type {
         Undefined,
         DDS,
         NIF,
         Form,
      };
      Q_ENUM(Type);

   protected:
      Type    _type = Type::Undefined;
      QString _path;
      struct {
         std::mutex load_state;
      } _locks;
      struct {
         struct {
            uint32_t all           = 0;
            uint32_t other_assets  = 0;
            uint32_t render_window = 0;
         } refcounts;
         bool load_requested      = false; // prevents the asset manager from queuing multiple threads to load the same asset
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

      void on_handle_made(DovahKitAssetReceptor&);
      void on_handle_lost(DovahKitAssetReceptor&);

   public:
      DovahKitAsset(QString path, Type, QObject* parent = nullptr);
      virtual ~DovahKitAsset();

      inline QString path() const noexcept { return this->_path; }
      inline Type type() const noexcept { return this->_type; }

      inline bool areDependenciesLoaded() const noexcept { return this->_state.dependencies_loaded; }
      inline bool didContentLoadingFail() const noexcept { return this->_state.content_failed; }
      inline bool isContentLoaded() const noexcept { return this->_state.content_loaded; }
      inline bool isReady() const noexcept { return this->isContentLoaded() && this->areDependenciesLoaded(); }

      inline const DirectX::ScratchImage* ddsImage()    const noexcept { return this->_data.dds.data; }
      inline const DirectX::TexMetadata*  ddsMetadata() const noexcept { return this->_data.dds.info; }

      inline QImage image() const noexcept { return this->_data.image; }

   protected slots:
      void load();
      void unload();

   signals:
      void contentLoaded();        // The asset's own content has loaded.
      void contentLoadingFailed(); // The asset's own content has failed to load.
      void dependenciesLoaded();   // The asset depends on other assets, and they have all finished loading.
      void ready();                // The asset is ready for use: its own content and the content of any dependencies is all loaded.
};

class DovahKitAssetTransport {
   friend class DovahKitAssetReceptor;
   protected:
      DovahKitAsset* value = nullptr;
   public:
      DovahKitAssetTransport() {}
      DovahKitAssetTransport(DovahKitAsset* v) : value(v) {}
      ~DovahKitAssetTransport();

      DovahKitAssetTransport(const DovahKitAssetTransport&) = delete;
      DovahKitAssetTransport& operator=(const DovahKitAssetTransport&) = delete;

      DovahKitAssetTransport(DovahKitAssetTransport&&) noexcept;
      DovahKitAssetTransport& operator=(DovahKitAssetTransport&&) noexcept;
};

class DovahKitAssetReceptor : public QObject {
   Q_OBJECT;
   public:
      using value_type = DovahKitAsset;
      enum Flag {
         IsRenderWindow = 0x00000001, // This handle is being used by the Render Window's 3D view.
      };
      Q_DECLARE_FLAGS(Flags, Flag);
      Q_FLAG(Flags);
   protected:
      struct state_flag {
         state_flag() = delete;
         enum type : uint8_t {
            ready  = 0x01,
            failed = 0x02,
         };
      };
      using state_flags_t = std::underlying_type_t<state_flag::type>;

      void _severLoadSignals();
      
   protected slots:
      void _forwardFailed();
      void _forwardReady();
      void _forwardUnloaded(QObject* target);

   protected:
      QPointer<value_type> asset;
      state_flags_t state = 0;

      void _acquire(value_type* asset);
      void _clear();

   public:
      DovahKitAssetReceptor() {}
      DovahKitAssetReceptor(Flags f) : flags(f) {}
      DovahKitAssetReceptor(DovahKitAssetTransport&& v);
      DovahKitAssetReceptor(const DovahKitAssetReceptor& other);
      DovahKitAssetReceptor(DovahKitAssetReceptor&& other) noexcept;
      ~DovahKitAssetReceptor();

      Flags flags = 0;

      operator bool() { return this->asset != nullptr; };
      operator value_type*() const noexcept { return this->asset.data(); };
      value_type* operator->() const noexcept { return this->asset.data(); };

      DovahKitAssetReceptor& operator=(DovahKitAssetTransport&& target) noexcept;
      DovahKitAssetReceptor& operator=(const DovahKitAssetReceptor& other) noexcept;
      DovahKitAssetReceptor& operator=(DovahKitAssetReceptor&& other) noexcept;

      inline value_type* bare() const noexcept { return this->asset.data(); }

      bool isFailed() const noexcept;
      bool isReady() const noexcept;

   signals:
      void failed();
      void ready();
      void unloaded();
};