#pragma once
#include <cassert>
#include <cstdint>
#include <functional>
#include "memory.h"
#include "../../zlib/zlib.h"

namespace cobb {
   namespace zlib {
      typedef std::function<size_t(uint32_t, cobb::generic_buffer&)> decompress_stream_getter;
      int decompress_stream(void* destination, uint32_t destination_size, uint32_t stream_size, decompress_stream_getter getter) {
         //
         // DO NOT USE
         //
         // this works some of the time, but only some of the time, and i have no earthly idea why
         //
         // the zlib api is extremely unintuitive, and the official examples come with sixteen paragraphs 
         // of text that doesn't actually explain anything (e.g. if the official code has two loops, with 
         // the outer handling refilling of the input buffer and the inner handling filling of the output 
         // buffer, and if the inner loop only terminates when there is unused/leftover output, then that 
         // implies that we refill the input buffer not when it's actually empty, but when we have output 
         // that we apparently can't use; why is this the case? if the inner loop is also responsible for 
         // reusing the output buffer, then that would imply that in cases where we've actually malloc'd 
         // enough space for all decompressed data in advance, we should not have an inner loop, no? and 
         // yet things are breaking here. given that the example program is the sort of thing that we 
         // have to be able to write in order to actually use zlib, how is it that the official guide to 
         // the example program consists of literally over a dozen paragraphs of useless minutiae and no 
         // actual information on how one would modify the presented code to cover different use cases?)
         //
         // so yeah, i have no clue how to fix this one. copying the compressed data into a buffer works 
         // well enough, so i'll probably just delete this
         //
         cobb::generic_buffer buffer(stream_size);
         z_stream strm;
         int ret;
         //
         strm.zalloc   = nullptr;
         strm.zfree    = nullptr;
         strm.opaque   = nullptr;
         strm.avail_in = 0;
         strm.next_in  = nullptr;
         ret = inflateInit(&strm);
         if (ret != Z_OK)
            return ret;
         //
         // ZLIB works as follows:
         //
         //  - The outer loop attempts to fill an input buffer.
         //
         //  - The inner loop attempts to write to an output buffer.
         //
         //  - The library halts if there is no more input to decompress, or if 
         //    there is no more room for output.
         //
         do {
            strm.avail_in  = getter(strm.total_in, buffer);
            strm.avail_out = destination_size - strm.total_in;
            strm.next_out  = (Bytef*)destination + strm.total_in;
            if (strm.avail_in == 0)
               break;
            strm.next_in = (Bytef*)buffer.raw();
            //
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
               case Z_NEED_DICT:
                  ret = Z_DATA_ERROR; /* and fall through */
               case Z_DATA_ERROR:
               case Z_MEM_ERROR:
                  (void)inflateEnd(&strm);
                  return ret;
            }
         } while (ret != Z_STREAM_END);
         //
         (void)inflateEnd(&strm);
         return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
      }
   }
}
