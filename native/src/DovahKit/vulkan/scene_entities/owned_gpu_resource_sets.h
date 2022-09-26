#pragma once
#include <concepts>
#include <optional>

namespace vulkanDK::scene_entities {
   template<typename Data>
   struct owned_gpu_resource_sets {
      public:
         using data_type = Data;

      protected:
         static constexpr const bool can_query_emptiness = requires(const data_type& data) {
            { data.empty() } -> std::same_as<bool>;
         };
         static constexpr const bool can_cast_to_bool = requires(const data_type& data) {
            { (bool)data };
         };
         static constexpr const bool can_invert_to_bool = requires(const data_type& data) {
            { !!data } -> std::same_as<bool>;
         };

         static bool _data_exists(const data_type& data) requires (can_query_emptiness || can_cast_to_bool || can_invert_to_bool) {
            if constexpr (can_query_emptiness) {
               return !data.empty();
            } else if constexpr (can_cast_to_bool) {
               return (bool)data;
            } else if constexpr (can_invert_to_bool) {
               return !!data;
            }
         }

      public:
         using stored_type = std::conditional_t<
            (can_query_emptiness || can_cast_to_bool || can_invert_to_bool),
            data_type,
            std::optional<data_type>
         >;

      public:
         owned_gpu_resource_sets() {}
         owned_gpu_resource_sets(const owned_gpu_resource_sets&) = delete;
         owned_gpu_resource_sets(owned_gpu_resource_sets&& o) noexcept { *this = std::move(o); }

         owned_gpu_resource_sets& operator=(const owned_gpu_resource_sets&) = delete;
         owned_gpu_resource_sets& operator=(owned_gpu_resource_sets&& o) noexcept {
            std::swap(this->current,  o.current);
            std::swap(this->outdated, o.outdated);
            return *this;
         }

      public:
         stored_type current;
         stored_type outdated;

         inline bool has_current() const { return _data_exists(current); }
         inline bool has_outdated() const { return _data_exists(outdated); }

         void destroy_current() {
            this->current = std::move(data_type());
         }
         void destroy_outdated() {
            this->outdated = std::move(data_type());
         }
   };
}