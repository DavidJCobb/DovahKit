#include "pixel_format.h"
#include <algorithm>
#include "helpers/generic_reader_ex.h"
#include "load_exception.h"

namespace vulkanDK::dds {
   void pixel_format::read(cobb::generic_reader_ex& reader) {
      reader.require_size(serialized_size);
      reader.unchecked_read(this->size);
      if (this->size != pixel_format::serialized_size)
         throw load_exception("Invalid pixel format (serialized size is wrong).");
      reader.unchecked_read(this->flags);
      reader.unchecked_read<std::endian::big>(this->four_cc);
      reader.unchecked_read(this->rgb_bitcount);
      reader.unchecked_read(this->channel_masks.list);
   }

   std::array<std::pair<int8_t, int>, 4> pixel_format::channel_sequence() const noexcept {
      using pair_type = std::pair<int8_t, int>;

      std::array<pair_type, 4> positions = std::array{ pair_type{ 0, 33 }, pair_type{ 1, 33 }, pair_type{ 2, 33 }, pair_type{ 3, 33 } };
      for (size_t i = 0; i < 4; ++i) {
         auto mask = this->channel_masks.list[i];
         if (!mask)
            continue;
         auto least_significant_bit_pos = std::bit_width(mask & -(int32_t)mask);
         positions[i].second = least_significant_bit_pos;
      }
      std::sort(positions.begin(), positions.end(), [](const auto& a, const auto& b) {
         if (a.second >= 32)
            return false;
         return a.second > b.second;
      });
      for (auto& item : positions) {
         if (item.second >= 32)
            item.first = -1;
      }
      return positions;
   }
}