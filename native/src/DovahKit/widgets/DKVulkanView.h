#pragma once
#include <chrono>
#include <QBasicTimer>
#include <QWidget>
#include <QWindow>
#if !defined(QT_PLUGIN)
   #include "vulkan/scene_entity_handle.h"
#endif

namespace vulkanDK {
   class surface_renderer;
}

class DKVulkanView : public QWidget {
   Q_OBJECT;
   public:
      DKVulkanView(QWidget* parent = nullptr);
      ~DKVulkanView();

      virtual QPaintEngine* paintEngine() const override;

      inline QString preferredGPUName() const noexcept { return this->preferred_gpu.name; }
      constexpr bool requireExactGPUMatch() const noexcept { return this->preferred_gpu.exactMatch; }

      constexpr uint desiredFrameDelay() const noexcept { return this->desired_frame_delay_ms; }

      constexpr vulkanDK::surface_renderer* surfaceRenderer() const { return this->renderer; }

      constexpr bool isListeningForInput() const {
         return this->input_handling.enabled && this->input_handling.focused;
      }

   public slots:
      void setDesiredFrameDelay(uint ms);
      void setPreferredGPUName(const QString&); // not retroactive; use before supplying an instance
      void setRequireExactGPUMatch(bool); // not retroactive; use before supplying an instance

      void resetRenderer(); // re-selects a physical device, etc.

      void setInputHandlingEnabled(bool);

   signals:
      void rendererReady();
      void rendererTeardownImminent();
      void rendererTeardownComplete();

      #if !defined(QT_PLUGIN)
         void renderedMeshClicked(vulkanDK::rendered_mesh_handle);
      #endif

      void rendererErrorKillImminent(vulkanDK::surface_renderer&);
      void rendererKilledDueToError();

   protected:
      vulkanDK::surface_renderer* renderer = nullptr; // owns
      struct {
         bool    exactMatch = false;
         QString name;
      } preferred_gpu;
      uint desired_frame_delay_ms = 0;
      QBasicTimer timer;
      struct {
         bool enabled = false;
         bool focused = false;
      } input_handling;
      bool renderer_killed_due_to_error = false;

      #if !defined(QT_PLUGIN)
      void _inputPoll();
      void _killRendererDueToError();
      #endif

      virtual bool event(QEvent*) override;
      virtual void hideEvent(QHideEvent* event) override;
      virtual void paintEvent(QPaintEvent* event) override;
      virtual void resizeEvent(QResizeEvent* event) override;
      virtual void showEvent(QShowEvent* event) override;
      virtual void timerEvent(QTimerEvent* event) override;

      virtual void focusInEvent(QFocusEvent* event) override;
      virtual void focusOutEvent(QFocusEvent* event) override;
};