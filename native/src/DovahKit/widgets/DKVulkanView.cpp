#include "DKVulkanView.h"
#include <QEvent>
#include <QMouseEvent>
#include "../vulkan/data/DKVulkanCameraUpdate.h"
#include "../vulkan/DKVulkanInstance.h"
#include "../vulkan/physical_device.h"
#include "../vulkan/queue_family_info.h"
#include "../vulkan/surface_renderer.h"
#include "../dk3d/DK3DInputHandler.h"

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
   //
   // Get instance; set up surface:
   //
   auto& dkvi = DKVulkanInstance::get();
   QObject::connect(&dkvi, &QObject::destroyed, this, []() {
      assert(false && "A DKVulkanView and surface_renderer should never outlive the DKVulkanInstance!");
   });
   this->resetRenderer();
}
DKVulkanView::~DKVulkanView() {
   if (auto* s = this->renderer) {
      delete s;
      this->renderer = nullptr;
   }
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

void DKVulkanView::resetRenderer() {
   if (this->renderer) {
      delete this->renderer;
      this->renderer = nullptr;
   }
   //
   auto& dkvi = DKVulkanInstance::get();
   this->renderer = new vulkanDK::surface_renderer(dkvi, this);
   //
   const vulkanDK::physical_device* pd = nullptr;
   int32_t pd_score = 0;
   //
   const auto& list_pd = dkvi.physicalDevices();
   for (const auto* current : list_pd) {
      bool is_preferred = false;
      {
         auto& pn = this->preferred_gpu.name;
         if (!pn.isEmpty()) {
            auto current_name = QString::fromUtf8((const char*)current->info.name.c_str(), current->info.name.size());
            is_preferred = pn == current_name;
            if (!is_preferred && this->preferred_gpu.exactMatch)
               continue;
         }
      }
      if (!current->has_extensions(device_extensions))
         continue;
      {
         auto ssi = current->surface_support_details(this->renderer->handle);
         if (ssi.formats.empty())
            continue;
         if (ssi.presentation_modes.empty())
            continue;
      }
      {  // Check for queue support
         auto f = vulkanDK::queue_family_info(current->handle, this->renderer->handle);
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
         current_score = current->score();
      if (current_score > pd_score) {
         pd_score = current_score;
         pd       = current;
      }
   }
   if (!pd) {
      qDebug("[DKVulkanView] Failed to find a physical device that supports this surface.");
      return;
   }
   //
   // Found a device. Set up our renderer.
   //
   this->renderer->set_physical_device(*pd);
}

void DKVulkanView::setInputHandlingEnabled(bool e) {
   if (this->input_handling.enabled == e)
      return;
   this->input_handling.enabled = e;
   this->setFocusPolicy(e ? Qt::ClickFocus : Qt::NoFocus);
   if (!e)
      this->input_handling.focused = false;
}

void DKVulkanView::_inputPoll() {
   if (!this->input_handling.enabled || !this->input_handling.focused)
      return;
   auto* s = this->renderer;
   if (!s)
      return;
   auto update = DK3DInputHandler::get().update(this);
   s->scene.adjust_camera(update);
}

#pragma region Events
bool DKVulkanView::event(QEvent* event) {
   if (event->type() == QEvent::Type::WinIdChange) {
      if (auto* s = this->renderer) {
         s->update_widget_id();
      }
   }
   return QWidget::event(event);
}
void DKVulkanView::hideEvent(QHideEvent* event) {
   this->killTimer(this->timerID);
   this->timerID = 0;
   //
   if (auto* s = this->renderer) {
      s->_on_visibility_change(QSize(), false);
   }
}
void DKVulkanView::paintEvent(QPaintEvent* event) {
   if (auto* s = this->renderer) {
      s->_on_repaint();
   }
}
void DKVulkanView::resizeEvent(QResizeEvent* event) {
   if (auto* s = this->renderer) {
      s->_on_visibility_change(this->size(), this->isVisible());
   }
}
void DKVulkanView::showEvent(QShowEvent* event) {
   this->timerID = this->startTimer(this->desiredFrameDelay(), Qt::PreciseTimer);
   //
   if (auto* s = this->renderer) {
      s->_on_visibility_change(this->size(), this->isVisible());
   }
}
void DKVulkanView::timerEvent(QTimerEvent* event) {
   this->_inputPoll();
   this->repaint();
}

void DKVulkanView::focusInEvent(QFocusEvent* event) {
   this->input_handling.focused = true;
   DK3DInputHandler::get().viewFocusChange(this, true);
}
void DKVulkanView::focusOutEvent(QFocusEvent* event) {
   this->input_handling.focused = false;
   DK3DInputHandler::get().viewFocusChange(this, false);
}

void DKVulkanView::mousePressEvent(QMouseEvent* event) {
   auto* s = this->renderer;
   if (!s)
      return;
   auto pos = event->localPos();
   auto i   = s->object_index_at(pos.x(), pos.y());
   if (i == -1)
      return;
   emit this->renderedMeshClicked(i);
}
#pragma endregion