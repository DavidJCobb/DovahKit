#pragma once
#include <QWidget>

namespace vulkanDK {
   class surface_renderer;
}

class DKVulkanView : public QWidget {
   Q_OBJECT;
   public:
      DKVulkanView(QWidget* parent = nullptr);
      ~DKVulkanView();

      virtual QPaintEngine* paintEngine() const override { return nullptr; }

      inline QString preferredGPUName() const noexcept { return this->preferred_gpu.name; }
      inline bool requireExactGPUMatch() const noexcept { return this->preferred_gpu.exactMatch; }

      inline uint desiredFrameDelay() const noexcept { return this->desired_frame_delay_ms; }

      inline vulkanDK::surface_renderer* surfaceRenderer() const { return this->renderer; }

   public slots:
      void setDesiredFrameDelay(uint ms);
      void setPreferredGPUName(const QString&); // not retroactive; use before supplying an instance
      void setRequireExactGPUMatch(bool); // not retroactive; use before supplying an instance

      void resetRenderer(); // re-selects a physical device, etc.

   signals:
      void rendererReady();
      void rendererTeardownImminent();
      void rendererTeardownComplete();

      void renderedMeshClicked(size_t);

   protected:
      vulkanDK::surface_renderer* renderer = nullptr; // owns
      struct {
         bool    exactMatch = false;
         QString name;
      } preferred_gpu;
      uint desired_frame_delay_ms = 0;
      int  timerID = 0;

      virtual bool event(QEvent*) override;
      virtual void hideEvent(QHideEvent* event) override;
      virtual void paintEvent(QPaintEvent* event) override;
      virtual void resizeEvent(QResizeEvent* event) override;
      virtual void showEvent(QShowEvent* event) override;
      virtual void timerEvent(QTimerEvent* event) override;

      virtual void mousePressEvent(QMouseEvent* event) override;
};