#pragma once
#include <QWidget>

class DKVulkanInstance;
namespace vulkanDK {
   class surface;
}

class DKVulkanView : public QWidget {
   Q_OBJECT;
   public:
      DKVulkanView(QWidget* parent = nullptr);
      ~DKVulkanView();

      inline DKVulkanInstance* instance() const noexcept { return this->vulkan_instance; }

      inline QString preferredGPUName() const noexcept { return this->preferred_gpu.name; }
      inline bool requireExactGPUMatch() const noexcept { return this->preferred_gpu.exactMatch; }

      inline uint desiredFrameDelay() const noexcept { return this->desired_frame_delay_ms; }

   public slots:
      void setInstance(DKVulkanInstance*);

      void setDesiredFrameDelay(uint ms);
      void setPreferredGPUName(const QString&); // not retroactive; use before supplying an instance
      void setRequireExactGPUMatch(bool); // not retroactive; use before supplying an instance

   protected:
      DKVulkanInstance*  vulkan_instance = nullptr;
      vulkanDK::surface* vulkan_surface  = nullptr;
      struct {
         bool    exactMatch = false;
         QString name;
      } preferred_gpu;
      uint desired_frame_delay_ms = 0;
      int  timerID = 0;

      virtual void hideEvent(QHideEvent* event) override;
      virtual void paintEvent(QPaintEvent* event) override;
      virtual void resizeEvent(QResizeEvent* event) override;
      virtual void showEvent(QShowEvent* event) override;
      virtual void timerEvent(QTimerEvent* event) override;
};