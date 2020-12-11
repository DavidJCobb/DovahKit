#pragma once
#include "file_threaded_part_loader_base.h"

namespace dovah::tes_file_reading::threads {
   class basic : public file_threaded_part_loader_base {
      //
      // Class for reading a top-level GRUP for a form type that cannot contain child 
      // GRUPs.
      //
      public:
         static constexpr int recommended_thread_count = 4;
         static constexpr int heavy_duty_thread_count  = 0;
         //
      protected:
         struct queued_group {
            uint32_t signature = 0;
            uint32_t pos       = 0;
            //
            queued_group(uint32_t s, uint32_t p) : signature(s), pos(p) {}
         };
         //
         virtual void exec() override final;
         //
      public:
         using file_threaded_part_loader_base::file_threaded_part_loader_base;
         //
         std::vector<queued_group> queue;
         //
         void add_group(uint32_t groupSignature, uint32_t groupPos);
   };
   class dialogue : public file_threaded_part_loader_base {
      public:
         static constexpr int recommended_thread_count = 1;
         static constexpr int heavy_duty_thread_count  = 0;
         //
      protected:
         struct queued_group {
            uint32_t signature = 0;
            uint32_t pos       = 0;
            //
            queued_group(uint32_t s, uint32_t p) : signature(s), pos(p) {}
         };
         //
         virtual void exec() override final;
         //
      public:
         using file_threaded_part_loader_base::file_threaded_part_loader_base;
         //
         std::vector<queued_group> queue;
         //
         void add_group(uint32_t groupSignature, uint32_t groupPos);
   };
   class interior_cell : public file_threaded_part_loader_base {
      public:
         static constexpr int recommended_thread_count = 4;
         static constexpr int heavy_duty_thread_count  = 0;
         //
      protected:
         struct queued_block {
            uint32_t blockNumber = 0;
            uint32_t pos = 0;
            //
            queued_block(uint32_t bn, uint32_t p) : blockNumber(bn), pos(p) {}
         };
         //
         virtual void exec() override final;
         //
      public:
         using file_threaded_part_loader_base::file_threaded_part_loader_base;
         //
         std::vector<queued_block> queue;
         //
         void add_group(uint32_t groupSignature, uint32_t groupPos);
   };
   class worldspace_sub_block : public file_threaded_part_loader_base {
      public:
         static constexpr int recommended_thread_count = 6;
         static constexpr int heavy_duty_thread_count  = 0;
         //
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
         virtual void exec() override final;
         //
      public:
         using file_threaded_part_loader_base::file_threaded_part_loader_base;
         //
         std::vector<queued_sub_block> queue;
         //
         void add_group(uint32_t worldID, int16_t bx, int16_t by, int16_t sbx, int16_t sby, uint32_t pos);
   };
   class worldspace_persistent_cell_children : public file_threaded_part_loader_base {
      public:
         static constexpr int recommended_thread_count = 2;
         static constexpr int heavy_duty_thread_count  = 0;
         //
      protected:
         struct queued_group {
            uint32_t cellID;
            uint32_t pos;
            //
            queued_group(uint32_t c, uint32_t p) : cellID(c), pos(p) {};
         };
         //
         virtual void exec() override final;
         //
      public:
         using file_threaded_part_loader_base::file_threaded_part_loader_base;
         //
         std::vector<queued_group> queue;
         //
         void add_group(uint32_t cellID, uint32_t pos);
   };
   class game_setting : public file_threaded_part_loader_base {
      //
      // Class for reading a top-level GRUP of GMST.
      //
      public:
         static constexpr int recommended_thread_count = 1;
         static constexpr int heavy_duty_thread_count  = 0;
         //
      protected:
         struct queued_group {
            uint32_t pos = 0;
            //
            queued_group(uint32_t p) : pos(p) {}
         };
         //
         virtual void exec() override final;
         //
      public:
         using file_threaded_part_loader_base::file_threaded_part_loader_base;
         //
         std::vector<queued_group> queue;
         //
         void add_group(uint32_t groupPos);
   };
}