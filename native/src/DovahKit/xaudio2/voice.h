#pragma once
#include <cstdint>
#include <vector>
class  IXAudio2Voice;
struct XAUDIO2_EFFECT_DESCRIPTOR;
struct XAUDIO2_FILTER_PARAMETERS;
struct XAUDIO2_SEND_DESCRIPTOR;

namespace dovahkit::xaudio2 {
   class voice {
      protected:
         IXAudio2Voice* _voice = nullptr;
         struct {
            uint32_t channel_count = 0;
            uint32_t effect_count  = 0;
         } _details;

      protected:
         void _set_nth_effect_parameters(uint32_t n, const void* data, uint32_t size);
         
      #pragma region Constructors and destructor
      protected:
         voice();
      public:
         voice(const voice&) = delete;
         voice(voice&&) noexcept;

         ~voice();
      #pragma endregion

      public:
         constexpr IXAudio2Voice* get_raw_interface() {
            return this->_voice;
         }

         constexpr uint32_t get_channel_count() const noexcept {
            return this->_details.channel_count;
         }
         constexpr uint32_t get_effect_count() const noexcept {
            return this->_details.effect_count;
         }

         float get_volume() const;
         void set_volume(float);

         std::vector<float> get_all_channel_volumes() const;
         void set_all_channel_volumes(const std::vector<float>&);

         void clear_effect_chain();
         void replace_effect_chain(const std::vector<XAUDIO2_EFFECT_DESCRIPTOR>&);

         template<typename Params>
         void set_nth_effect_parameters(uint32_t n, const Params& params) {
            this->_set_nth_effect_parameters(n, &params, sizeof(Params));
         }

         void set_filter_parameters(const XAUDIO2_FILTER_PARAMETERS&);

         void clear_destination_voices();
         void set_destination_voices(const std::vector<XAUDIO2_SEND_DESCRIPTOR>&);
         void set_mastering_voice_as_sole_destination();
   };
}