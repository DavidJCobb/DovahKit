#pragma once
#include <array>
#include <bit>
#include <QObject>
#include <QPointer>
#include <QWidget>
#include <glm/glm.hpp>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "../helpers/intrusive_windows_defines.h"

// Currently only usable for the render window's 3D widget, but then, that goes for this 
// whole file right now. We'll figure out multiple 3D views later, I'm sure.
class DovahKitVulkanWidget : public QWidget {
   Q_OBJECT;
   public:
      DovahKitVulkanWidget(QWidget* parent = nullptr);

      virtual QPaintEngine* paintEngine() const override { return nullptr; }

   protected:
      virtual void hideEvent(QHideEvent* event) override;
      virtual void paintEvent(QPaintEvent *event) override;
      virtual void resizeEvent(QResizeEvent* event) override;
      virtual void showEvent(QShowEvent* event) override;
      virtual void timerEvent(QTimerEvent* event) override;

      int timerID = 0;
};

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

      struct frame_in_flight {
         VkFence fence;
         struct {
            VkSemaphore image_available;
            VkSemaphore render_finished;
         } semaphores;
      };

      struct vertex {
         glm::vec2 pos;
         glm::vec3 color;
         //
         static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
         static VkVertexInputBindingDescription getBindingDescription();
      };

      bool initialized = false;
      bool failed      = false;
      VkInstance       instance;
      VkRenderPass     render_pass;
      VkPipelineLayout pipeline_layout;
      VkPipeline       pipeline;
      VkBuffer         vertex_buffer;
      VkDeviceMemory   vertex_buffer_memory;
      VkBuffer         index_buffer;
      VkDeviceMemory   index_buffer_memory;
      VkCommandPool    command_pool;
      std::vector<VkCommandBuffer> command_buffers;
      struct {
         VkSwapchainKHR handle;
         VkFormat       format;
         VkExtent2D     extent;
         std::vector<VkImage>       images;
         std::vector<VkImageView>   views;
         std::vector<VkFramebuffer> framebuffers;
         std::vector<VkFence>       images_in_flight; // handles. if swap_chain.images[i] is in flight, then swap_chain.images_in_flight[i] == frames_in_flight[x].fence; else, it's a null handle
      } swap_chain;
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
            //
            QSize last_size;
            bool  resized = false;
            bool  visible = false;
         } render_window;
      } surfaces;
      std::vector<frame_in_flight> frames_in_flight;
      size_t current_frame = 0;
      VkDebugUtilsMessengerEXT debugMessenger;
      //
      const std::vector<vertex> vertices = {
         {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
         {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
         {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
         {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}},
      };
      const std::vector<uint16_t> indices = {
         0, 1, 2, 2, 3, 0
      };

      static VkDebugUtilsMessengerCreateInfoEXT _get_debug_create_params();

      static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
         VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
         VkDebugUtilsMessageTypeFlagsEXT messageType,
         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
         void* pUserData
      );

      void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
      void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
      int32_t deviceScore(VkPhysicalDevice) const;
      uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

      VkShaderModule createShaderModule(const QByteArray compiled_shader);

      void setupInstance();
      void setupDebugMessenger();
      void setupRenderWindowSurface();
      void setupPhysicalDevice();
      void setupLogicalDevice();
      void setupSwapChain();
      void setupImageViews();
      void setupRenderPass();
      void setupGraphicsPipeline();
      void setupFramebuffers();
      void setupCommandPool();
      void setupVertexBuffer();
      void setupIndexBuffer();
      void setupCommandBuffers();
      void setupSemaphores();

      void recreateSwapChain();

      void teardownSwapChain();

   public:
      void initialize(); // TODO: do stuff here instead of in the constructor
      void teardown();

      inline QWidget* renderWindowWidget() const noexcept { return this->surfaces.render_window.widget; }
      inline VkSurfaceKHR& renderWindowSurface() noexcept { return this->surfaces.render_window.surface; }

      inline bool isInitialized() const noexcept { return this->initialized; }

      void drawFrame();
      void renderWindowStateChange(QSize, bool visible);

   signals:
      void ready();
      void teardownImminent();
      void teardownComplete();
};