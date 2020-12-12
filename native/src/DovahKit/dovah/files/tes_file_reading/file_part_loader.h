#pragma once
#include "file_or_file_part_loader.h"

namespace dovah::tes_file_reading {
   class file_loader;

   class file_part_loader : public file_or_file_part_loader {
      //
      // Base class for loading part of a file; it is owned by a (file_loader) and should 
      // be managed and destroyed by that (file_loader). A (file_loader) can use multiple 
      // (file_part_loader)s  to load different  parts of a file  asynchronously with one 
      // another.
      //
      friend class file_loader;
      protected:
         virtual file_loader& get_file_loader() const noexcept override final { return *this->loader; }
      public:
         file_part_loader(file_loader&);
         //
         bool load_record_at(uint32_t pos);
         //
         object_type next_record_or_group(); // only called during the initial file read
         bool        next_subrecord(); // called after the initial file read, when loading a form_stub's full content
         //
         void _ensure_file();
         void _on_file_close(); // called by file_loader::close
         //
      protected:
         using basic_reader::loader; // make this field protected. also, for this class, it should never be nullptr
   };
}