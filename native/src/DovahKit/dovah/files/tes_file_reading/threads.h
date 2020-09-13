#pragma once
#include <thread>
#include <vector>
#include "basic_reader.h"

namespace dovah {
   namespace tes_file_reading {
      namespace threads {
         class basic : public basic_reader {
            //
            // Class for reading a top-level GRUP for a form type that cannot contain child 
            // GRUPs.
            //
            protected:
               struct queued_group {
                  uint32_t signature = 0;
                  uint32_t pos       = 0;
                  //
                  queued_group(uint32_t s, uint32_t p) : signature(s), pos(p) {}
               };
               //
               void _load();
               static void _thread_handler(basic* instance);
            public:
               basic(file_reader& f) : basic_reader(&f) {}
               basic(file_reader& f, bool allowNestedGroups) : basic_reader(&f), allow_nested_groups(allowNestedGroups) {}

               bool allow_nested_groups = false;
               std::vector<queued_group> queue;
               std::thread thread;
               //
               void add_group(uint32_t groupSignature, uint32_t groupPos);
               void start();
               void wait_for();
         };
         class interior_cell : public basic_reader {
            protected:
               struct queued_block {
                  uint32_t blockNumber = 0;
                  uint32_t pos = 0;
                  //
                  queued_block(uint32_t bn, uint32_t p) : blockNumber(bn), pos(p) {}
               };
               //
               void _load();
               static void _thread_handler(interior_cell* instance);
            public:
               interior_cell(file_reader& f) : basic_reader(&f) {}
               //
               std::vector<queued_block> queue;
               std::thread thread;
               //
               void add_group(uint32_t groupSignature, uint32_t groupPos);
               void start();
               void wait_for();
         };
         class worldspace_sub_block : public basic_reader {
            protected:
               struct queued_sub_block {
                  uint32_t worldspaceID = 0;
                  int16_t  blockX = 0;
                  int16_t  blockY = 0;
                  int16_t  subBlockX = 0;
                  int16_t  subBlockY = 0;
                  uint32_t pos = 0;
                  //
                  queued_sub_block(uint32_t a, int16_t b, int16_t c, int16_t d, int16_t e, uint32_t f) : worldspaceID(a), blockX(b), blockY(c), subBlockX(d), subBlockY(e), pos(f) {};
               };
               //
               void _load();
               static void _thread_handler(worldspace_sub_block* instance);
            public:
               worldspace_sub_block(file_reader& f) : basic_reader(&f) {}
               //
               std::vector<queued_sub_block> queue;
               std::thread thread;
               //
               void add_group(uint32_t worldID, int16_t bx, int16_t by, int16_t sbx, int16_t sby, uint32_t pos);
               void start();
               void wait_for();
         };
         class worldspace_persistent_cell_children : public basic_reader {
            protected:
               struct queued_group {
                  uint32_t cellID;
                  uint32_t pos;
                  //
                  queued_group(uint32_t c, uint32_t p) : cellID(c), pos(p) {};
               };
               //
               void _load();
               static void _thread_handler(worldspace_persistent_cell_children* instance);
            public:
               worldspace_persistent_cell_children(file_reader& f) : basic_reader(&f) {}
               //
               std::vector<queued_group> queue;
               std::thread thread;
               //
               void add_group(uint32_t cellID, uint32_t pos);
               void start();
               void wait_for();
         };
      }
   }
}