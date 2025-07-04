#pragma once
#include <optional> // std::nullopt_t
#include <type_traits>

namespace cobb {
   template<typename RefType> requires std::is_lvalue_reference_v<RefType>
   class optional_ref {
      public:
         using reference  = RefType;
         using value_type = std::remove_reference<reference>;

      private:
         value_type* _data = nullptr;

      public:
         constexpr optional_ref() noexcept {}
         constexpr optional_ref(std::nullopt_t) noexcept {}
         constexpr optional_ref(const optional_ref& other) : _data(other._data) {}
         constexpr optional_ref(optional_ref&& other) : _data(other._data) {}
         constexpr optional_ref(reference target) : _data(std::addressof(target)) {}

         // In-place construction makes no sense for an optional reference. Disallow it.
         template<typename... Args>
         constexpr explicit optional_ref(std::in_place_t, Args&&...) = delete;
         template<typename T, typename... Args>
         constexpr explicit optional(std::in_place_t, std::initializer_list<T>, Args&&...) = delete;

         constexpr const value_type* operator->() const noexcept { return this->_data; }
         constexpr value_type* operator->() noexcept { return this->_data; }
         constexpr const reference operator*() const noexcept { return *this->_data; }
         constexpr reference operator*() noexcept { return *this->_data; }

         constexpr explicit operator bool() const noexcept { return this->has_value(); }
         constexpr bool has_value() const noexcept { return this->_data != nullptr; }

         constexpr reference value() { return *this->_data; }
         constexpr const reference value() const { return *this->_data; }

         template<class T = std::remove_cv_t<reference>>
         constexpr value_type value_or(T&& default_value) const = delete;

         // Emplacement makes no sense for an optional reference. Disallow it.
         template<typename... Args>
         constexpr reference emplace(Args&&...) = delete;
         template<typename T, typename... Args>
         constexpr reference emplace(std::initializer_list<T>, Args&&...) = delete;

         constexpr void reset() { this->_data = nullptr; }
   };
}