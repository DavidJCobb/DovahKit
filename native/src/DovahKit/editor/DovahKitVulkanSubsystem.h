#pragma once
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <QObject>
#include <QPointer>
#include <QWidget>
#include <glm/glm.hpp>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "../helpers/intrusive_windows_defines.h"
#include "../vulkan/descriptor_definitions.h"

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

   public:
      struct vertex {
         glm::vec3 pos;
         glm::vec3 color;
         glm::vec2 texCoord;
         //
         static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
         static VkVertexInputBindingDescription getBindingDescription();
      };

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

      struct shader_module {
         //
         // You can technically discard this after you've created the graphics pipelines that 
         // will use it, but keeping it around will avoid the need to reload your compiled 
         // shaders every time you rebuild the swap chain.
         //
         VkDevice       device = VK_NULL_HANDLE;
         VkShaderModule handle = VK_NULL_HANDLE;

         shader_module(VkDevice, const QByteArray&);
         ~shader_module();

         shader_module(const shader_module&) = delete;
         shader_module& operator=(const shader_module&) = delete;
         shader_module(shader_module&&) noexcept;
         shader_module& operator=(shader_module&&) noexcept;
      };

      struct frame_in_flight {
         VkFence fence;
         struct {
            VkSemaphore image_available;
            VkSemaphore render_finished;
         } semaphores;
      };
      
      struct uniform_buffer_object {
         //
         // Vulkan expects precise member aligmnent; see: <https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/chap15.html#interfaces-resources-layout>
         //
         alignas(16) glm::mat4 view;
         alignas(16) glm::mat4 proj;
      };
      struct push_constant {
         int32_t texture_index;
         int32_t object_index;
      };

      struct loaded_texture {
         VkImage     image = VK_NULL_HANDLE;
         VkImageView view  = VK_NULL_HANDLE;
         VkDeviceMemory memory = VK_NULL_HANDLE; // TODO: in the future, multiple textures should share a single VkDeviceMemory via suballocation
         //
         uint32_t w;
         uint32_t h;
         QString  path;
         //
         bool     pending_delete    = false; // unhook the texture from frames' descriptors; delete it when it's fully unhooked
         uint32_t frame_dirty_flags = -1; // for normal textures: frames that need descriptors resynchronized. for pending-delete textures: frames that may still be using the texture in their descriptors.
         uint32_t refcount          =  0;
      };
      struct rendered_object {
         protected:
            void _on_shader_parameter_change();
         public:
            struct shader_parameters { // pass to the shader via a storage buffer
               glm::mat4 transform;
            };
            //
            struct {
               VkBuffer       buffer = VK_NULL_HANDLE;
               VkDeviceMemory memory = VK_NULL_HANDLE; // TODO: in the future, buffers should share VkDeviceMemory allocations via suballocations (we need to build a custom CPU-side heap for GPU memory, basically)
               uint32_t       indices_at     = 0;
               uint32_t       index_count    = 0;
               VkDeviceSize   allocated_size = 0; // TODO: when we improve buffer management, this will be queryable from the buffer wrapper
            } vertex_and_index_buffer;
            shader_parameters shader_params;
            uint32_t frame_dirty_flags = -1; // for normal objects: frames that need (shader_params) resynchronized. for pending-delete objects: frames that may still be using the vertex-and-index buffer
            bool     pending_delete    = false; // unhook the object's vertex-and-index buffer from frames' command buffers; delete it when it's fully unhooked
            int32_t  texture_index     = -1;
            //
            inline bool empty() const noexcept { return this->vertex_and_index_buffer.buffer == VK_NULL_HANDLE; }
            //
            inline const glm::mat4& transform() const noexcept { return this->shader_params.transform; }
            void set_transform(const glm::mat4&);
      };

      struct rendered_object_animation_state {
         bool  playing  = true;
         float duration = 4.0;
         float elapsed  = 0.0;
      };

      bool initialized = false;
      bool failed      = false;
      struct {
         bool  null_descriptors      = false; // are we allowed to set descriptors to VK_NULL_HANDLE (presuming we also enable logical-device-side support)?
         float anisotropic_filtering = 0;     // max supported value
      } support;
      VkInstance       instance;
      VkRenderPass     render_pass;
      std::vector<shader_module> shader_modules;
      vulkanDK::descriptor_set_layout descriptor_set_layout;
      VkDescriptorPool                descriptor_pool;
      std::vector<VkDescriptorSet>    descriptor_sets;
      VkPipelineLayout pipeline_layout;
      VkPipeline       pipeline;
      VkCommandPool    command_pool;
      std::vector<VkCommandBuffer> command_buffers;
      std::vector<bool> command_buffer_is_out_of_date;
      VkSampler        texture_sampler;
      struct {
         VkImage        image;
         VkDeviceMemory memory;
         VkImageView    view;
      } depth_buffer;
      struct {
         VkImage        image;
         VkDeviceMemory memory; // TODO: should share; implement a buffer allocator
         VkImageView    view;
      } null_texture; // for if the nullDescriptor feature isn't supported/enabled
      struct {
         VkSwapchainKHR handle;
         VkFormat       format;
         VkExtent2D     extent;
         std::vector<VkImage>        images;
         std::vector<VkImageView>    views;
         std::vector<VkFramebuffer>  framebuffers;
         std::vector<VkBuffer>       uniform_buffers;
         std::vector<VkDeviceMemory> uniform_buffer_memory;
         //
         struct {
            std::vector<VkBuffer>       buffer_handles;
            std::vector<VkDeviceMemory> buffer_memory;
         } rendered_object_shader_parameters;
         //
         std::vector<VkFence> images_in_flight; // handles. if swap_chain.images[i] is in flight, then swap_chain.images_in_flight[i] == frames_in_flight[x].fence; else, it's a null handle
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
      struct {
         std::vector<loaded_texture> textures;
      } assets;
      std::vector<rendered_object> rendered_objects;
      std::vector<rendered_object_animation_state> anim_state;
      std::chrono::steady_clock::time_point last_update;

      static VkDebugUtilsMessengerCreateInfoEXT _get_debug_create_params();

      static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
         VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
         VkDebugUtilsMessageTypeFlagsEXT messageType,
         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
         void* pUserData
      );

      VkCommandBuffer beginSingleTimeCommands();
      void endSingleTimeCommands(VkCommandBuffer single_time_command_buffer);

      void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize);
      void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
      VkImageView createImageView(VkImage, VkFormat, VkImageAspectFlags) const;
      void createVkImage(uint32_t w, uint32_t h, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImage& out_image, VkDeviceMemory& out_memory) const;
      int32_t deviceScore(VkPhysicalDevice) const;
      uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags) const;
      VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling, VkFormatFeatureFlags) const;

      VkFormat findDepthFormat() const;

      void copyBufferToImage(VkBuffer, VkImage, uint32_t width, uint32_t height);
      void transitionImageLayout(VkImage, VkFormat, VkImageLayout oldLayout, VkImageLayout newLayout);

      VkShaderModule createShaderModule(const QByteArray compiled_shader);

      void setupInstance();
      void setupDebugMessenger();
      void setupRenderWindowSurface();
      void setupPhysicalDevice();
      void setupLogicalDevice();
      void setupShaderModules();
      void setupSwapChain();
      void setupImageViews();
      void setupRenderPass();
      void setupDescriptorSetLayout();
      void setupGraphicsPipeline();
      void setupFramebuffers();
      void setupCommandPool();
      void setupDepthBuffer();
      void setupTextures();
      void setupTextureSampler();
      void setupRenderedObjects();
      void setupShaderParameterBuffers();
      void setupDescriptorPool();
      void setupDescriptorSets();
      void setupCommandBuffers();
      void setupSemaphores();

      void recreateSwapChain();

      void teardownSwapChain();

      void refillCommandBuffers(size_t which_frame);

      size_t insertNewLoadedTexture();  // grabs the first free entry in the list, or creates one. returns std::string::npos if no slot available.
      size_t insertNewRenderedObject(); // grabs the first free entry in the list, or creates one. returns std::string::npos if no slot available.
      void deleteRenderedObject(size_t index);

   public:
      void initialize(); // TODO: do stuff here instead of in the constructor
      void teardown();

      inline QWidget* renderWindowWidget() const noexcept { return this->surfaces.render_window.widget; }
      inline VkSurfaceKHR& renderWindowSurface() noexcept { return this->surfaces.render_window.surface; }

      inline bool isInitialized() const noexcept { return this->initialized; }

      void updateShaderParameterBuffers(uint32_t which);
      void updateShaderTextureDescriptors(uint32_t which);
      void drawFrame();
      void renderWindowStateChange(QSize, bool visible);

      void setAnimationPaused(size_t, bool);
      void updateAnimationState();

      // For testing:
      size_t addTexture(const QString& texture_path); // returns std::string::npos if loading fails. does NOT increment the texture refcount; do that yourself!
      void addRenderedObject(const QString& texture_path); // load a texture and add a quad with it
      void removeRenderedObject(); // remove the last rendered object in the list

   signals:
      void ready();
      void teardownImminent();
      void teardownComplete();
};