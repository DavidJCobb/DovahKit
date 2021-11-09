#pragma once
#include <QPointer>
#include <QWidget>
#include "widgets/DKVulkanView.h"
#include "_vulkan.h"
#include "_util.h"

class DKVulkanInstance;

namespace vulkanDK {
   class surface_renderer;

   class surface : no_copy, only_heap_allocate {
      friend class DKVulkanView;
      friend class surface_renderer;
      protected:
         QPointer<DKVulkanView> _widget;
         WId _lastWidgetID = {};

         DKVulkanInstance* _owner    = nullptr;
         surface_renderer* _renderer = nullptr; // owned
         struct {
            bool resized = false;
            bool visible = false;
         } state;

         void _setup();
         void _teardown();

         // renderer events:
         void _on_renderer_ready();
         void _on_renderer_teardown_imminent();
         void _on_renderer_teardown_complete();

         // widget events:
         void _on_repaint();
         void _on_resize();
         void _on_visibility_change(QSize, bool visible);

      public:
         surface(DKVulkanInstance&);
         surface(DKVulkanInstance&, DKVulkanView*);
         ~surface();

         surface(surface&&) noexcept;
         surface& operator=(surface&&) noexcept;

         VkSurfaceKHR handle = VK_NULL_HANDLE;

         inline DKVulkanInstance* owner() const noexcept { return this->_owner; }
         
         inline DKVulkanView* widget() const noexcept { return this->_widget; }
         void setWidget(DKVulkanView*);
         void updateWidgetID();
   };
}