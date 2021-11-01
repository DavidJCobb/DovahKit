#pragma once
#include <array>
#include <bit>
#include <QObject>
#include <QPointer>
#include <QWidget>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "../helpers/intrusive_windows_defines.h"

class DovahKitVulkanSubsystem final : public QObject {
   Q_OBJECT;
   public:
      static DovahKitVulkanSubsystem& get() {
         static DovahKitVulkanSubsystem instance;
         return instance;
      }
   protected:
      DovahKitVulkanSubsystem();
      ~DovahKitVulkanSubsystem();

      class QueueFamilies {
         public:
            using queue_index_t = uint32_t;

         protected:
            void set(queue_index_t& entry, queue_index_t value);

         public:
            union {
               struct {
                  queue_index_t graphics;
                  queue_index_t presentation;
               };
               std::array<queue_index_t, 2> list;
            } families;
            uint8_t mask = 0;

            static constexpr size_t         unique_family_count = std::tuple_size_v<decltype(families.list)>;
            static constexpr decltype(mask) all_mask_bits_set   = (1 << unique_family_count) - 1;

            QueueFamilies(VkPhysicalDevice);
            bool has(const queue_index_t& entry) const noexcept;

            inline bool has_index(size_t i) const noexcept {
               return (this->mask & (1 << i)) != 0;
            }
         
            static_assert(sizeof(families) == sizeof(families.list), "The array doesn't include all families. Did you add some without increasing the array length to match?");
            static_assert(sizeof(mask) * 8 >= std::bit_width(unique_family_count), "The mask type is not large enough to track all values. Make it bigger.");
      };

      struct swap_chain_support_info {
         VkSurfaceCapabilitiesKHR        capabilities;
         std::vector<VkSurfaceFormatKHR> formats;
         std::vector<VkPresentModeKHR>   presentation_modes;
         //
         swap_chain_support_info() {}
         swap_chain_support_info(VkPhysicalDevice, VkSurfaceKHR);
      };

      bool initialized = false;
      bool failed      = false;
      VkInstance     instance;
      VkSwapchainKHR swap_chain;
      struct {
         VkPhysicalDevice physical = VK_NULL_HANDLE;
         VkDevice logical;
      } devices;
      struct {
         VkQueue graphics;
         VkQueue presentation;
      } queues;
      struct {
         struct {
            VkSurfaceKHR      surface;
            QPointer<QWidget> widget;
         } render_window;
      } surfaces;
      VkDebugUtilsMessengerEXT debugMessenger;

      static VkDebugUtilsMessengerCreateInfoEXT _get_debug_create_params();

      static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
         VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
         VkDebugUtilsMessageTypeFlagsEXT messageType,
         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
         void* pUserData
      );

      int32_t deviceScore(VkPhysicalDevice) const;

      void setupInstance();
      void setupDebugMessenger();
      void setupRenderWindowSurface();
      void setupPhysicalDevice();
      void setupLogicalDevice();
      void setupSwapChain();

   public:
      void initialize(); // TODO: do stuff here instead of in the constructor
      void teardown();

      inline QWidget* renderWindowWidget() const noexcept { return this->surfaces.render_window.widget; }
      inline VkSurfaceKHR& renderWindowSurface() noexcept { return this->surfaces.render_window.surface; }

   signals:
      void ready();
      void teardownImminent();
      void teardownComplete();
};