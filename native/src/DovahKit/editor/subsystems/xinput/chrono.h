#pragma once
#include <chrono>

namespace dovahkit::subsystems::xinput {
   using timestamp_t = std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds>;
   static constexpr auto zero_time = timestamp_t(timestamp_t::duration::zero());
}
