//
// Created by Tomasz Rudzki on 02/07/2024.
//

#include "../iamfbr/src/iamfbr_impl.h"
#include "extras.h"

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

        switch (plugin_state.selected_preset_id) {
          case 1:
            input_type = iamfbr::InputType::k714;
            break;
            //          case 2:
            //            input_type = iamfbr::InputType::k51;
            //            break;
        }
        break;
      case InputType::Individual_sources:
        DBG("Individual sources not supported yet.");
        return;
    }

    iamfbr_ = std::make_unique<iamfbr::IamfbrImpl>(buffer_size_per_channel,
                                                   sampling_rate, input_type);
  }

  void update_source(int input_channel, float gain, float azimuth,
                     float elevation, float distance) {
    // Setup iamfbr.
  }

  void deinitialize() { iamfbr_.reset(); }

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

    iamfbr_->process_planar_audio_data(input_buffer, output_buffer);

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
  std::unique_ptr<iamfbr::IamfbrImpl> iamfbr_;
};
