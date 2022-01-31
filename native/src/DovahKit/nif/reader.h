#pragma once
#include <cassert>
#include <string>
#include "helpers/generic_reader.h"
#include "file.h"
#include "blocks/_factory.h"
#include "types/NiMatrix33.h"
#include "types/NiTransform.h"

namespace nifDK {
   class file;

   class file_reader : public cobb::generic_reader {
      protected:
         file* subject = nullptr;
      public:
         file_reader(file* o, const void* d, size_t s) : cobb::generic_reader(d, s), subject(o) {}

         using cobb::generic_reader::generic_reader::read;
         using cobb::generic_reader::generic_reader::unchecked_read;

         bool read(NiMatrix33&);
         void unchecked_read(NiMatrix33&);
         bool read(NiTransform&);
         void unchecked_read(NiTransform&);

         template<typename Desired> bool read_ref(Desired*& out) {
            out = nullptr;
            if (!this->is_in_bounds(4))
               return false;
            int32_t index;
            this->unchecked_read(index);
            if (index == -1) // sentinel for "None"
               return true;
            //
            // NOTE: Refs should always point down the hierarchy; other types exist for back-references.
            //
            assert(this->subject && "You shouldn't be attempting to read refs except from a file's reader.");
            auto* instance = this->subject->block_by_index(index);
            if (instance == nullptr)
               return false;
            auto* casted = dynamic_cast<Desired>(instance);
            if (!casted)
               return false;
            out = casted;
            return true;
         }

         bool read(file_version& out) {
            if (!this->is_in_bounds(4))
               return false;
            this->unchecked_read(out);
            return true;
         }
         void unchecked_read(file_version& out) {
            this->read(&out.value, 4);
         }

         bool read_line_string(std::string& out) {
            uint8_t byte;
            bool    line = false;
            while (this->read(byte)) {
               if (byte == '\n' || byte == '\00') {
                  break;
               }
               if (byte == '\r') {
                  line = true;
                  continue;
               }
               if (line)
                  return false; // '\r' not followed by '\n'
               out.push_back(byte);
            }
            return true;
         }

         template<typename T> bool read_vector(std::vector<T>& out) {
            if (!this->is_in_bounds(out.size() * sizeof(T)))
               return false;
            this->unchecked_read(out.data(), out.size() * sizeof(T));
            return true;
         }

         template<int N, typename T, glm::qualifier Q> bool read_vector_vector(std::vector<glm::vec<N, T, Q>>& out) {
            constexpr size_t size = sizeof(T) * N;
            if (!this->is_in_bounds(out.size() * size))
               return false;
            this->unchecked_read(out.data(), out.size() * size);
            return true;
         }
   };
}