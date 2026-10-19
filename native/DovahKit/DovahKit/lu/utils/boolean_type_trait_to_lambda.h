#pragma once

namespace lu::utils {
   template<template<typename> typename Trait>
   constexpr const auto boolean_type_trait_to_lambda = []<typename Element>() -> bool {
      return Trait<Element>::value;
   };
}