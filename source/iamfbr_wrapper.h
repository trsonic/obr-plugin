//
// Created by Tomasz Rudzki on 02/07/2024.
//

#include "ambisonic_encoder.h"
#include "extras.h"
#include "iamfbr_impl.h"

#ifndef IAMFBR_IAMFBR_WRAPPER_H
#define IAMFBR_IAMFBR_WRAPPER_H

#endif  // IAMFBR_IAMFBR_WRAPPER_H

class iamfbrWrapper {
 public:
  iamfbrWrapper() {}

  void update_DSP_config(size_t buffer_size_per_channel, int sampling_rate,
                         const plugin_state& plugin_state) {
    iamfbr::InputType input_type;
    switch (plugin_state.input_type) {
      case InputType::Ambisonics:
        encoder_.reset();
        switch (plugin_state.ambisonic_order) {
          case 3:
            input_type = iamfbr::InputType::k3OA;
            break;
          case 7:
            input_type = iamfbr::InputType::k7OA;
            break;
        }
        break;
      case InputType::Loudspeaker_feeds:
        encoder_.reset();

        switch (plugin_state.selected_preset_id) {
          case 1:
            input_type = iamfbr::InputType::k714;
            break;
          case 2:
            input_type = iamfbr::InputType::k51;
            break;
        }
        break;
      case InputType::Individual_sources:
        encoder_ = std::make_unique<iamfbr::ambisonic_encoder>(
            buffer_size_per_channel, 64, plugin_state.ambisonic_order);

        input_type = iamfbr::InputType::k7OA;
        break;
    }

    iamfbr_ = std::make_unique<iamfbr::iamfbr_impl>(buffer_size_per_channel,
                                                    sampling_rate, input_type);
  }

  void update_source(int input_channel, float gain, float azimuth,
                     float elevation, float distance) {
    if (encoder_ == nullptr) return;
    encoder_->set_source(input_channel, gain, azimuth, elevation, distance,
                         iamfbr::SHCalculationMethod::Generate);
  }

  void deinitialize() {
    encoder_.reset();
    iamfbr_.reset();
  }

  void process(juce::AudioBuffer<float>& buffer) {
    if (!iamfbr_) return;

    // get the number of channels
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    // declare float buffers
    std::vector<float> input_buffer(
        static_cast<size_t>(numSamples * numChannels), 0.0f);
    std::vector<float> output_buffer(
        static_cast<size_t>(numSamples * numChannels), 0.0f);

    // copy data from buffer to input_buffer
    for (int channel = 0; channel < numChannels; ++channel) {
      const float* source = buffer.getReadPointer(channel);
      std::copy(source, source + numSamples,
                input_buffer.begin() + channel * numSamples);
    }

    if (encoder_ != nullptr) {
      // buffer to store encoded Ambisonic scene
      // get number of output channels from the encoder
      size_t num_output_channels = encoder_->get_number_of_output_channels();

      std::vector<float> intermediate_buffer(
          static_cast<size_t>(numSamples * num_output_channels), 0.0f);

      encoder_->process_planar_audio_data(input_buffer, intermediate_buffer);
      iamfbr_->process_planar_audio_data(intermediate_buffer, output_buffer);

    } else {
      iamfbr_->process_planar_audio_data(input_buffer, output_buffer);
    }

    // copy data from output_buffer to buffer
    for (int channel = 0; channel < numChannels; ++channel) {
      float* dest = buffer.getWritePointer(channel);
      std::copy(output_buffer.begin() + channel * numSamples,
                output_buffer.begin() + (channel + 1) * numSamples, dest);
    }

    //        // clear remaining channels
    //        for (auto i = number_of_ears; i < buffer.getNumChannels(); ++i)
    //          buffer.clear(i, 0, buffer.getNumSamples());
  }

 private:
  std::unique_ptr<iamfbr::ambisonic_encoder> encoder_;
  std::unique_ptr<iamfbr::iamfbr_impl> iamfbr_;
};
