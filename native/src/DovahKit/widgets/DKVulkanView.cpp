#include "DKVulkanView.h"
#include "../vulkan/DKVulkanInstance.h"
#include "../vulkan/logical_device.h"
#include "../vulkan/physical_device.h"
#include "../vulkan/queue_family_info.h"
#include "../vulkan/surface.h"
#include "../vulkan/surface_renderer.h"

namespace {
   const std::vector<const char*> device_extensions = { // TODO: match this with the extension list we request in logical_device !
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
   };
}

DKVulkanView::DKVulkanView(QWidget* parent) : QWidget(parent) {
   this->setAttribute(Qt::WA_OpaquePaintEvent, true);
   this->setAttribute(Qt::WA_PaintOnScreen,    true);
   this->winId(); // force the widget to have a unique HWND
}
DKVulkanView::~DKVulkanView() {
   if (auto* s = this->vulkan_surface) {
      delete s;
      this->vulkan_surface = nullptr;
   }
}

void DKVulkanView::setInstance(DKVulkanInstance* dkvi) {
   if (this->vulkan_instance != dkvi) {
      if (auto* s = this->vulkan_surface) {
         delete s;
         this->vulkan_surface = nullptr;
      }
   } else {
      assert(this->vulkan_surface == nullptr);
   }
   this->vulkan_instance = dkvi;
   this->vulkan_surface  = new vulkanDK::surface(*dkvi, this);
   //
   const vulkanDK::physical_device* pd = nullptr;
   int32_t pd_score = 0;
   //
   const auto& list_pd = dkvi->physicalDevices();
   for (auto& current : list_pd) {
      bool is_preferred = false;
      {
         auto& pn = this->preferred_gpu.name;
         if (!pn.isEmpty()) {
            auto current_name = QString::fromUtf8((const char*)current.info.name.c_str(), current.info.name.size());
            is_preferred = pn == current_name;
            if (!is_preferred && this->preferred_gpu.exactMatch)
               continue;
         }
      }
      if (!current.has_extensions(device_extensions))
         continue;
      {
         auto ssi = current.surface_support_details(this->vulkan_surface->handle);
         if (ssi.formats.empty())
            continue;
         if (ssi.presentation_modes.empty())
            continue;
      }
      {  // Check for queue support
         auto f = vulkanDK::queue_family_info(current.handle, *this->vulkan_surface);
         if (!f.has(f.families.graphics)) // require this queue family type
            continue;
         if (!f.has(f.families.presentation)) // require this queue family type
            continue;
      }
      //
      int32_t current_score = 0;
      if (is_preferred)
         current_score = std::numeric_limits<decltype(current_score)>::max();
      else
         current_score = current.score();
      if (current_score > pd_score) {
         pd_score = current_score;
         pd       = &current;
      }
   }
   if (!pd) {
      qDebug("[DKVulkanView::setInstance] Failed to find a physical device that supports this surface.");
      return;
   }
   //
   // Found a device. Set up our renderer.
   //
   auto* ld       = new vulkanDK::logical_device(*dkvi, *pd);
   auto  renderer = new vulkanDK::surface_renderer(*this->vulkan_surface, *ld);
   this->vulkan_surface->_renderer = renderer;
}

void DKVulkanView::setDesiredFrameDelay(uint ms) {
   this->desired_frame_delay_ms = ms;
   if (this->timerID) {
      this->killTimer(this->timerID);
      this->timerID = this->startTimer(this->desiredFrameDelay(), Qt::PreciseTimer);
   }
}
void DKVulkanView::setPreferredGPUName(const QString& name) {
   this->preferred_gpu.name = name;
}
void DKVulkanView::setRequireExactGPUMatch(bool em) {
   this->preferred_gpu.exactMatch = em;
}

void DKVulkanView::hideEvent(QHideEvent* event) {
   this->killTimer(this->timerID);
   this->timerID = 0;
   //
   if (auto* s = this->vulkan_surface) {
      s->_on_visibility_change(QSize(), false);
   }
}
void DKVulkanView::paintEvent(QPaintEvent* event) {
   if (auto* s = this->vulkan_surface) {
      s->_on_repaint();
   }
}
void DKVulkanView::resizeEvent(QResizeEvent* event) {
   if (auto* s = this->vulkan_surface) {
      s->_on_visibility_change(this->size(), this->isVisible());
   }
}
void DKVulkanView::showEvent(QShowEvent* event) {
   this->timerID = this->startTimer(this->desiredFrameDelay(), Qt::PreciseTimer);
   //
   if (auto* s = this->vulkan_surface) {
      s->_on_visibility_change(this->size(), this->isVisible());
   }
}
void DKVulkanView::timerEvent(QTimerEvent* event) {
   this->repaint();
}