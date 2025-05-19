#include "DKVulkanView.h"
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#if !defined(QT_PLUGIN)
   #include "editor/subsystems/worldedit/core.h"
   #include "vulkan/data/DKVulkanCameraUpdate.h"
   #include "vulkan/DKVulkanInstance.h"
   #include "vulkan/exceptions.h"
   #include "vulkan/physical_device.h"
   #include "vulkan/queue_family_info.h"
   #include "vulkan/surface_renderer.h"
#endif

namespace {
   constexpr int null_icon_render_bounds = 400;
   constexpr int null_icon_render_size   = null_icon_render_bounds * 0.66;
   constexpr int null_icon_line_width    = std::max(10, (int)((float)null_icon_render_bounds * 0.07));
}

DKVulkanView::DKVulkanView(QWidget* parent) : QWidget(parent) {
   this->setAttribute(Qt::WA_OpaquePaintEvent, true);
   this->setAttribute(Qt::WA_PaintOnScreen,    true);
   this->winId(); // force the widget to have a unique HWND
   //
   // Get instance; set up surface:
   //
   #if !defined(QT_PLUGIN)
      auto& dkvi = DKVulkanInstance::get();
      QObject::connect(&dkvi, &QObject::destroyed, this, []() {
         assert(false && "A DKVulkanView and surface_renderer should never outlive the DKVulkanInstance!");
      });
      this->resetRenderer();
   #endif
}
DKVulkanView::~DKVulkanView() {
   #if !defined(QT_PLUGIN)
      if (auto* s = this->renderer) {
         delete s;
         this->renderer = nullptr;
      }
   #endif
}


QPaintEngine* DKVulkanView::paintEngine() const {
   if (this->renderer_killed_due_to_error)
      return QWidget::paintEngine();
   return nullptr;
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
   #if !defined(QT_PLUGIN)
   if (this->renderer) {
      delete this->renderer;
      this->renderer = nullptr;
   }
   #endif
   if (this->renderer_killed_due_to_error) {
      this->setAttribute(Qt::WA_OpaquePaintEvent, true);
      this->setAttribute(Qt::WA_PaintOnScreen,    true);
   }
   this->renderer_killed_due_to_error = false;
   //
   #if !defined(QT_PLUGIN)
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
      if (!vulkanDK::surface_renderer::device_is_supported(*current))
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
      this->_killRendererDueToError();
      return;
   }
   //
   // Found a device. Set up our renderer.
   //
   try {
      this->renderer->set_physical_device(*pd);
   } catch (vulkanDK::exception& e) {
      this->_killRendererDueToError();
   }
   #endif
}

void DKVulkanView::setInputHandlingEnabled(bool e) {
   if (this->input_handling.enabled == e)
      return;
   this->input_handling.enabled = e;
   this->setFocusPolicy(e ? Qt::ClickFocus : Qt::NoFocus);
   if (!e)
      this->input_handling.focused = false;
}

#if !defined(QT_PLUGIN)
void DKVulkanView::_inputPoll() {
   if (!this->input_handling.enabled || !this->input_handling.focused)
      return;
   if (!this->renderer)
      return;
   dovahkit::subsystems::worldedit::core::get().view_input_poll_handler(*this);
}
void DKVulkanView::_killRendererDueToError() {
   this->renderer_killed_due_to_error = true;
   if (!this->renderer)
      return;
   emit this->rendererErrorKillImminent(*this->renderer);
   delete this->renderer;
   this->renderer = nullptr;
   this->setAttribute(Qt::WA_OpaquePaintEvent, false);
   this->setAttribute(Qt::WA_PaintOnScreen,    false);
   emit this->rendererKilledDueToError();
}
#endif

#pragma region Events
bool DKVulkanView::event(QEvent* event) {
   if (event->type() == QEvent::Type::WinIdChange) {
      #if !defined(QT_PLUGIN)
      if (auto* s = this->renderer) {
         s->update_widget_id();
      }
      #endif
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
   if (!this->renderer) {
      if (this->renderer_killed_due_to_error) {
         QPainter painter(this);
         auto rect = this->contentsRect();
         //
         constexpr int  margin    = null_icon_line_width  + 2;
         constexpr auto halfwidth = null_icon_render_size / 2;
         //
         QPointF center = rect.center();
         auto    length = std::min(rect.width(), rect.height()) - margin;
         //
         QColor color = this->palette().color(QPalette::ColorRole::Dark);
         QPen   pen;
         pen.setWidth(null_icon_line_width);
         pen.setColor(color);
         pen.setCapStyle(Qt::PenCapStyle::FlatCap);
         //
         painter.save();
         painter.setBrush(Qt::NoBrush);
         painter.setPen(pen);
         painter.setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);
         painter.translate(center);
         if (length < null_icon_render_bounds) {
            auto scale = qreal(length) / null_icon_render_bounds;
            painter.scale(scale, scale);
         }
         painter.drawLine( halfwidth, -halfwidth, -halfwidth,  halfwidth);
         painter.drawLine(-halfwidth, -halfwidth,  halfwidth,  halfwidth);
         painter.restore();
      }
      return;
   }
   #if !defined(QT_PLUGIN)
   try {
      this->renderer->_on_repaint();
   } catch (vulkanDK::exception& e) {
      this->_killRendererDueToError();
   }
   #endif
}
void DKVulkanView::resizeEvent(QResizeEvent* event) {
   #if !defined(QT_PLUGIN)
   if (auto* s = this->renderer) {
      s->_on_visibility_change(this->size(), this->isVisible());
   }
   #endif
}
void DKVulkanView::showEvent(QShowEvent* event) {
   this->timerID = this->startTimer(this->desiredFrameDelay(), Qt::PreciseTimer);
   //
   #if !defined(QT_PLUGIN)
   if (auto* s = this->renderer) {
      s->_on_visibility_change(this->size(), this->isVisible());
   }
   #endif
}
void DKVulkanView::timerEvent(QTimerEvent* event) {
   #if !defined(QT_PLUGIN)
   this->_inputPoll();
   #endif
   this->repaint();
}

void DKVulkanView::focusInEvent(QFocusEvent* event) {
   this->input_handling.focused = true;
}
void DKVulkanView::focusOutEvent(QFocusEvent* event) {
   this->input_handling.focused = false;
}
#pragma endregion