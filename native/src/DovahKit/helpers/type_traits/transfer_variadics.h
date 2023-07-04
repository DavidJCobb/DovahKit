/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#pragma once

namespace cobb {
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