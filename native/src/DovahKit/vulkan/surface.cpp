#include "surface.h"
#include "widgets/DKVulkanView.h"
#include "DKVulkanInstance.h"
#include "surface_renderer.h"

namespace vulkanDK {
   void surface::_setup() {
      assert(this->_owner);
      assert(this->_owner->available());
      assert(this->handle == VK_NULL_HANDLE);
      assert(this->_widget != nullptr);
      //
      this->_lastWidgetID = this->_widget->winId();
      //
      auto create_info = VkWin32SurfaceCreateInfoKHR{
         .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
         .hinstance = GetModuleHandle(nullptr),
         .hwnd      = (HWND)this->_lastWidgetID,
      };
      if (vkCreateWin32SurfaceKHR(this->_owner->getHandle(), &create_info, nullptr, &this->handle) != VK_SUCCESS) {
         this->handle        = VK_NULL_HANDLE;
         this->_lastWidgetID = {};
      }
   }
   void surface::_teardown() {
      if (this->handle == VK_NULL_HANDLE)
         return;
      assert(this->_owner);
      assert(this->_owner->available());
      //
      if (auto* r = this->_renderer) {
         r->teardown();
      }
      vkDestroySurfaceKHR(this->_owner->getHandle(), this->handle, nullptr);
      this->handle = VK_NULL_HANDLE;
   }

   surface::surface(DKVulkanInstance& dkvi) : _owner(&dkvi) {
   }
   surface::surface(DKVulkanInstance& dkvi, DKVulkanView* w) : _owner(&dkvi) {
      this->setWidget(w);
   }
   surface::~surface() {
      this->_teardown();
      if (this->_owner) {
         // TODO: Notify owner of our destruction
      }
   }

   surface::surface(surface&& o) noexcept {
      std::swap(this->_owner,  o._owner);
      std::swap(this->handle,  o.handle);
      std::swap(this->_widget, o._widget);
   }
   surface& surface::operator=(surface&& o) noexcept {
      std::swap(this->_owner,  o._owner);
      std::swap(this->handle,  o.handle);
      std::swap(this->_widget, o._widget);
      return *this;
   }

   void surface::setWidget(DKVulkanView* w) {
      if (this->_widget == w)
         return;
      this->_teardown();
      this->_widget = w;
      if (this->_owner && this->_owner->available())
         this->_setup();
   }
   void surface::updateWidgetID() {
      assert(this->_widget);
      if (this->_widget->winId() == this->_lastWidgetID)
         return;
      this->_teardown();
      if (this->_owner && this->_owner->available())
         this->_setup();
   }
}