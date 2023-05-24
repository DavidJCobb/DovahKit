#pragma once
#include <array>
#include <string_view>

namespace cobb {
   namespace impl::_name_of_type {
      template<typename T>
      constexpr auto function_name() {
         return std::string_view{
            #if defined(__clang__) || defined(__GNUC__)
               __PRETTY_FUNCTION__
            #elif defined (_MSC_VER)
               __FUNCSIG__
            #endif
         };
      }

      template<typename T>
      constexpr auto compute() {
         constexpr auto prefix = std::string_view(
            #if defined(__clang__)
               "[T = "
            #elif defined(__GNUC__)
               "with T = "
            #elif defined (_MSC_VER)
               "compute<"
            #endif
         );
         constexpr auto suffix = std::string_view(
            #if defined(__clang__)
               "]"
            #elif defined(__GNUC__)
               "]"
            #elif defined (_MSC_VER)
               ">("
            #endif
         );
         constexpr auto this_function = std::string_view{
            #if defined(__clang__) || defined(__GNUC__) //|| defined(__INTELLISENSE__)
               __PRETTY_FUNCTION__
            #elif defined (_MSC_VER)
               __FUNCSIG__
            #endif
         };

         constexpr size_t start = this_function.find(prefix) + prefix.size();
         constexpr size_t end   = this_function.rfind(suffix);
         static_assert(start < end);

         constexpr auto view  = this_function.substr(start, (end - start));

         std::array<char, (end - start)> out = {};
         for (size_t i = 0; i < out.size(); ++i)
            out[i] = view.data()[i];
         return out;
      }

      template<typename T>
      struct storage {
         static constexpr const auto value = compute<T>();
      };
   }

   // NOTE: IntelliSense often chokes on this when it's invoked a few templates deep. 
   //       You may need to check `#ifndef __INTELLISENSE__` when using it.
   template<typename T>
   constexpr std::string_view name_of_type() {
      constexpr const auto& arr = impl::_name_of_type::storage<T>::value;
      return std::string_view(arr.data(), arr.size());
   }
}