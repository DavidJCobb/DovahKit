#pragma once

namespace lu {
   // Given `Src`, an instantiation of a variadic template, instantiate another 
   // variadic template `DstTmp` with the same parameters.
   template<typename Src, template<typename...> typename DstTmp>
   struct transfer_variadics;

   template<template<typename...> typename SrcTmp, template<typename...> typename DstTmp, typename... SrcTypes>
   struct transfer_variadics<SrcTmp<SrcTypes...>, DstTmp> {
      using type = DstTmp<SrcTypes...>;

      template<typename... T>
      using append = DstTmp<SrcTypes..., T...>;

      template<typename... T>
      using prepend = DstTmp<T..., SrcTypes...>;
   };
}