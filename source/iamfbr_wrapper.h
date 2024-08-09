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

  void update_DSP(size_t buffer_size_per_channel, int sampling_rate,
                  int number_of_input_channels, plugin_state plugin_state) {
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
            buffer_size_per_channel, number_of_input_channels, plugin_state.ambisonic_order);
        break;
    }

    iamfbr_ = std::make_unique<iamfbr::iamfbr_impl>(buffer_size_per_channel,
                                                    sampling_rate, input_type);
  }

  void update_sources(std::vector<float> gains, std::vector<float> azimuths,
                      std::vector<float> elevations,
                      std::vector<float> distances) {
    for (size_t i = 0; i < azimuths.size(); i++) {
      encoder_->set_source(static_cast<int>(i), gains[i], azimuths[i],
                           elevations[i], 1.0f,
                           iamfbr::SHCalculationMethod::Generate);
    }
  }

  void deinitialize() {
    encoder_.reset();
    iamfbr_.reset();
  }

  void process(juce::AudioBuffer<float>& buffer) {
    // get the number of channels
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    int number_of_ears = 2;

    // declare float buffers
    std::vector<float> input_buffer(buffer.getNumSamples() *
                                    buffer.getNumChannels());

    std::vector<float> output_buffer(buffer.getNumSamples() * number_of_ears);

    // copy data from buffer to input_buffer
    for (int channel = 0; channel < numChannels; ++channel) {
      const float* source = buffer.getReadPointer(channel);
      std::copy(source, source + numSamples,
                input_buffer.begin() + channel * numSamples);
    }

    if (encoder_ && iamfbr_) {
      // buffer to store encoded Ambisonic scene
      // get number of output channels from the encoder
      size_t num_output_channels = encoder_->get_number_of_output_channels();
      std::vector<float> intermediate_buffer(buffer.getNumSamples() * num_output_channels);

      encoder_->process_planar_audio_data(input_buffer, intermediate_buffer);
      iamfbr_->process_planar_audio_data(intermediate_buffer, output_buffer);
    }
    else if (iamfbr_) {
        iamfbr_->process_planar_audio_data(input_buffer, output_buffer);
    }

    // copy data from output_buffer to buffer
    for (int channel = 0; channel < number_of_ears; ++channel) {
      float* dest = buffer.getWritePointer(channel);
      std::copy(output_buffer.begin() + channel * numSamples,
                output_buffer.begin() + (channel + 1) * numSamples, dest);
    }

    // clear remaining channels
    for (auto i = number_of_ears; i < buffer.getNumChannels(); ++i)
      buffer.clear(i, 0, buffer.getNumSamples());
  }

 private:
  std::unique_ptr<iamfbr::ambisonic_encoder> encoder_;
  std::unique_ptr<iamfbr::iamfbr_impl> iamfbr_;
};
